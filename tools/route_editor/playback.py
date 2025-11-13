import bpy
from mathutils import Vector
from .route_types import ROUTE_PARENT_TYPE_KEY, SEGMENT_MODE_KEY, SEGMENT_TIME_KEY

class ROUTE_OT_preview_modal(bpy.types.Operator):
    bl_idname = "route.preview_modal"
    bl_label = "ルートプレビュー（再生/一時停止/リセット）"

    action = bpy.props.EnumProperty(
        items=[("PLAY", "再生", ""), ("PAUSE", "一時停止", ""), ("RESET", "リセット", "")],
        default="PLAY"
    )

    _timer = None
    _running = False
    _t = 0.0
    _total = 0.0
    _route_parent = None
    _preview_obj = None

    def _collect_segments(self, parent):
        segs = [o for o in parent.children if o.type == 'MESH' and o.name.startswith("RouteSeg_")]
        def keyf(o):
            try:
                _, a, b = o.name.split("_")
                return int(a), int(b)
            except:
                return (0, 0)
        segs.sort(key=keyf)
        return segs

    def _collect_points(self, parent):
        pts = [o for o in parent.children if o.type == 'EMPTY' and o.name.startswith("RP_")]
        pts.sort(key=lambda o: int(o.name.split("_")[1]))
        return pts

    def _ensure_preview_obj(self, parent):
        name = parent.name + "_PreviewSquare"
        obj = bpy.data.objects.get(name)
        if obj is None:
            mesh = bpy.data.meshes.new(name + "_mesh")
            import bmesh
            bm = bmesh.new()
            s = 0.15
            vs = [(-s, -s, 0), (s, -s, 0), (s, s, 0), (-s, s, 0)]
            verts = [bm.verts.new(v) for v in vs]
            bm.faces.new(verts)
            bm.to_mesh(mesh)
            bm.free()
            obj = bpy.data.objects.new(name, mesh)
            bpy.context.scene.collection.objects.link(obj)
            obj.parent = parent
        self._preview_obj = obj

    def _lengths_and_times(self, segs):
        times = [float(seg.get(SEGMENT_TIME_KEY, 1.0)) for seg in segs]
        total = sum(times) if times else 0.0
        return times, total

    def modal(self, context, event):
        if event.type == 'TIMER' and self._running:
            if self._total <= 0.0:
                return {'PASS_THROUGH'}
            dt = 1.0 / context.scene.render.fps if context.scene.render.fps > 0 else 1/60
            self._t += dt
            if self._t >= self._total:
                self._t = self._total
                self._running = False

            segs = self._collect_segments(self._route_parent)
            points = self._collect_points(self._route_parent)
            times, _ = self._lengths_and_times(segs)

            acc = 0.0
            for i, tm in enumerate(times):
                if self._t <= acc + tm or i == len(times) - 1:
                    local_t = 0.0 if tm <= 0 else (self._t - acc) / tm
                    p0 = points[i].matrix_world.translation
                    p1 = points[i+1].matrix_world.translation
                    pos = p0.lerp(p1, local_t)
                    self._preview_obj.matrix_world.translation = pos
                    break
                acc += tm
            return {'RUNNING_MODAL'}
        return {'PASS_THROUGH'}

    def execute(self, context):
        parent = context.active_object
        if not (parent and parent.type == 'EMPTY' and ROUTE_PARENT_TYPE_KEY in parent):
            self.report({'ERROR'}, "ルート親(Empty)を選択してください")
            return {'CANCELLED'}

        self._route_parent = parent
        self._ensure_preview_obj(parent)

        segs = self._collect_segments(parent)
        _, total = self._lengths_and_times(segs)
        self._total = total

        if self.action == 'RESET':
            self._t = 0.0
            self._running = False
        elif self.action == 'PLAY':
            self._running = True
        elif self.action == 'PAUSE':
            self._running = False

        if self._timer is None:
            self._timer = context.window_manager.event_timer_add(0.0, window=context.window)
            context.window_manager.modal_handler_add(self)
        return {'RUNNING_MODAL'}

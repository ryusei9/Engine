import bpy
from bpy.props import FloatVectorProperty
from mathutils import Vector
from .route_types import (
 make_route_parent, make_point, make_segment_obj,
 ROUTE_PARENT_TYPE_KEY, ROUTE_TYPE_PLAYER, ROUTE_TYPE_CAMERA,
 SEGMENT_MODE_KEY, SEGMENT_TIME_KEY, SEGMENT_MODE_ITEMS,
)


# ルート作成（親Empty + 最初の点）
class ROUTE_OT_create_route(bpy.types.Operator):
    bl_idname = "route.create"
    bl_label = "ルート新規"

    # Blender 4.x 対応版：__annotations__でプロパティを登録
    __annotations__ = {
        "route_type": bpy.props.EnumProperty(
            name="Route Type",
            items=[
                ("PLAYER", "Player", "プレイヤー用ルート"),
                ("CAMERA", "Camera", "カメラ用ルート"),
            ],
            default="PLAYER"
        )
    }

    def execute(self, context):
        print("ルート作成:", self.route_type)
        # 実処理をここに
        return {'FINISHED'}

# route_types が無くても動くようフォールバック
try:
    from .route_types import SEGMENT_MODE_KEY, SEGMENT_TIME_KEY, SEGMENT_MODE_ITEMS
except Exception:
    SEGMENT_MODE_KEY = "segment_mode"
    SEGMENT_TIME_KEY = "segment_time"
    SEGMENT_MODE_ITEMS = [
        ('LINE', '直線', '直線で補間'),
        ('CURVE', '曲線', '曲線（Catmull-Rom 風）で補間'),
    ]

class ROUTE_OT_set_segment_props(bpy.types.Operator):
    """選択した線のモード/時間を設定"""
    bl_idname = "route.set_segment_props"
    bl_label = "選択した線のモード/時間を設定"
    bl_options = {'REGISTER', 'UNDO'}

    segment_mode = bpy.props.EnumProperty(
        name="線の種類",
        items=SEGMENT_MODE_ITEMS,
        default='LINE'
    )
    segment_time = bpy.props.FloatProperty(
        name="移動時間(秒)",
        default=1.0,
        min=0.0
    )

    @classmethod
    def poll(cls, context):
        obj = context.active_object
        # 線メッシュ（例：RouteSeg_001_002）を想定。厳密にしたくなければ type=='MESH' だけでもOK
        return obj and obj.type == 'MESH'

    def execute(self, context):
        seg = context.active_object
        seg[SEGMENT_MODE_KEY] = self.segment_mode
        seg[SEGMENT_TIME_KEY] = float(self.segment_time)

        # （任意）もし __init__.py で Object.segment_mode / segment_time を定義していれば同期
        if hasattr(seg, "segment_mode"):
            try:
                seg.segment_mode = self.segment_mode
            except Exception:
                pass
        if hasattr(seg, "segment_time"):
            try:
                seg.segment_time = float(self.segment_time)
            except Exception:
                pass

        self.report({'INFO'}, f"{seg.name}: mode={self.segment_mode}, time={self.segment_time:.3f}s")
        return {'FINISHED'}




# 最後の点から次の点を追加＆線オブジェクトを生成
class ROUTE_OT_add_point(bpy.types.Operator):
 bl_idname = "route.add_point"
 bl_label = "ルートに点を追加（線を接続）"
 bl_options = {'REGISTER', 'UNDO'}


 offset = FloatVectorProperty(name="オフセット", default=(1.0, 0.0, 0.0), subtype='XYZ')


 @classmethod
 def poll(cls, context):
  obj = context.active_object
  return obj and obj.type == 'EMPTY' and ROUTE_PARENT_TYPE_KEY in obj


def execute(self, context):
 parent = context.active_object
 # 子のうち、名前順で最後の点を取得
 points = [c for c in parent.children if c.type == 'EMPTY' and c.name.startswith("RP_")]
 points.sort(key=lambda o: o.name)
 if not points:
  self.report({'ERROR'}, "この親Emptyに点がありません。'ルートを新規作成'を使用してください")
  return {'CANCELLED'}
 last = points[-1]
 new_loc = last.location + Vector(self.offset)
 new_idx = len(points) + 1
 new_pt = make_point(f"RP_{new_idx:03d}", parent, new_loc)
 seg = make_segment_obj(f"RouteSeg_{new_idx-1:03d}_{new_idx:03d}", last, new_pt)
 segment_time = bpy.props.FloatProperty(name="移動時間(秒)", default=1.0, min=0.0)


@classmethod
def poll(cls, context):
 obj = context.active_object
 return obj and obj.type == 'MESH' and obj.name.startswith("RouteSeg_")


def execute(self, context):
 seg = context.active_object
 seg[SEGMENT_MODE_KEY] = self.segment_mode
 seg[SEGMENT_TIME_KEY] = float(self.segment_time)
 self.report({'INFO'}, f"{seg.name}: mode={self.segment_mode}, time={self.segment_time:.3f}s")
 return {'FINISHED'}




# 線の自動追従（点を動かしたら線メッシュの頂点座標を更新）
class ROUTE_OT_refresh_segments(bpy.types.Operator):
    bl_idname = "route.refresh_segments"
    bl_label = "ルートの線を更新"
    bl_options = {'REGISTER', 'UNDO'}

    @classmethod
    def poll(cls, context):
        obj = context.active_object
        return obj and obj.type == 'EMPTY' and ROUTE_PARENT_TYPE_KEY in obj

    def execute(self, context):
        parent = context.active_object
        segs = [o for o in parent.children if o.type == 'MESH' and o.name.startswith("RouteSeg_")]
        points = {o.name: o for o in parent.children if o.type == 'EMPTY' and o.name.startswith("RP_")}
        for seg in segs:
            try:
                _, a, b = seg.name.split("_")
                i0, i1 = a, b
            except:
                continue
            p0 = points.get(f"RP_{i0}")
            p1 = points.get(f"RP_{i1}")
            if not (p0 and p1):
                continue
            me = seg.data
            if len(me.vertices) >= 2:
                me.vertices[0].co = p0.location
                me.vertices[1].co = p1.location
                me.update()
        return {'FINISHED'}




classes_route = (
 ROUTE_OT_create_route,
 ROUTE_OT_add_point,
 ROUTE_OT_set_segment_props,
 ROUTE_OT_refresh_segments,
)
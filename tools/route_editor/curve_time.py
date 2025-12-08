import bpy

def ensure_time_array(obj):
    curve = obj.data
    point_count = sum(len(s.bezier_points) for s in curve.splines)

    # property が無ければ作る
    if "curve_point_time" not in curve:
        curve["curve_point_time"] = [1.0] * point_count
        return

    # 数が変わっていたら調整
    arr = list(curve["curve_point_time"])
    if len(arr) < point_count:
        arr.extend([1.0] * (point_count - len(arr)))
    elif len(arr) > point_count:
        arr = arr[:point_count]

    curve["curve_point_time"] = arr


class CURVE_PT_PointTimePanel(bpy.types.Panel):
    bl_label = "Curve Point Time"
    bl_idname = "CURVE_PT_point_time"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = "Route"

    def draw(self, context):
        layout = self.layout
        obj = context.object

        if not obj or obj.type != 'CURVE':
            layout.label(text="Select a curve object.")
            return

        ensure_time_array(obj)
        curve = obj.data
        times = curve["curve_point_time"]

        index = 0
        for spline in curve.splines:
            if spline.type != 'BEZIER':
                continue

            box = layout.box()
            box.label(text="Spline")

            for i, _ in enumerate(spline.bezier_points):
                row = box.row()
                row.prop(curve, f'["curve_point_time"]', index=index, text=f"Point {index}")
                index += 1


classes = (CURVE_PT_PointTimePanel,)


def register():
    for c in classes:
        bpy.utils.register_class(c)


def unregister():
    for c in reversed(classes):
        bpy.utils.unregister_class(c)

import bpy
from mathutils import Vector

class ROUTE_OT_add_curve_point(bpy.types.Operator):
    """選択中のカーブに新しい制御点を追加"""
    bl_idname = "route.add_curve_point"
    bl_label = "新規ポイント追加"
    bl_description = "選択中のベジエカーブに新しい制御点を追加します"

    offset = bpy.props.FloatVectorProperty(
        name="オフセット",
        description="新しく追加する点の相対位置",
        default=(1.0, 0.0, 0.0)
    )

    def execute(self, context):
        obj = context.active_object

        if not obj or obj.type != 'CURVE':
            self.report({'ERROR'}, "カーブオブジェクトを選択してください")
            return {'CANCELLED'}

        curve = obj.data
        spline = curve.splines.active if hasattr(curve.splines, "active") else curve.splines[0]


        if spline.type != 'BEZIER':
            self.report({'ERROR'}, "ベジエカーブのみ対応しています")
            return {'CANCELLED'}

        last_point = spline.bezier_points[-1]

        # オフセット適用（安全処理）
        try:
            offset_vec = Vector(self.offset[:])
        except:
            offset_vec = Vector((1.0, 0.0, 0.0))

        # 新しい点の位置（ワールド→ローカル）
        world_last = obj.matrix_world @ last_point.co
        world_new = world_last + offset_vec
        local_new = obj.matrix_world.inverted() @ world_new

        # 新規ポイント追加
        spline.bezier_points.add(1)
        new_point = spline.bezier_points[-1]
        new_point.co = local_new
        new_point.handle_left_type = 'AUTO'
        new_point.handle_right_type = 'AUTO'

        # --- ▼ 時間データ同期処理 ▼ ---
        point_count = len(spline.bezier_points)

        if "times" not in curve:
            curve["times"] = [1.0] * point_count  # 初回生成
        else:
            times = list(curve["times"])

            # 追加分を反映
            if len(times) < point_count:
                times.append(1.0)

            # 余分があれば切る
            elif len(times) > point_count:
                times = times[:point_count]

            curve["times"] = times
        # ------------------------------------

        curve.update_tag()
        context.view_layer.update()

        self.report({'INFO'}, f"ポイント追加 + time更新 ({len(curve['times'])} points)")
        return {'FINISHED'}
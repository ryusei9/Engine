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

        spline = obj.data.splines.active if hasattr(obj.data.splines, "active") else obj.data.splines[0]

        if spline.type != 'BEZIER':
            self.report({'ERROR'}, "ベジエカーブのみ対応しています")
            return {'CANCELLED'}

        last_point = spline.bezier_points[-1]

        # --- 安全なオフセット処理 ---
        offset_vec = Vector((1.0, 0.0, 0.0))
        try:
            if hasattr(self, "offset") and hasattr(self.offset, "__iter__"):
                offset_vec = Vector(self.offset[:])
        except Exception:
            pass
        # ----------------------------------

        # ワールド座標で新しい点の位置を計算
        world_last = obj.matrix_world @ last_point.co
        world_new = world_last + offset_vec
        # ローカル空間に戻す
        local_new = obj.matrix_world.inverted() @ world_new

        # 新しいポイントを追加
        spline.bezier_points.add(1)
        new_point = spline.bezier_points[-1]
        new_point.co = local_new
        new_point.handle_left_type = 'AUTO'
        new_point.handle_right_type = 'AUTO'

        obj.data.update_tag()
        bpy.context.view_layer.update()

        self.report({'INFO'}, f"新しいポイントを追加しました: {new_point.co}")
        return {'FINISHED'}

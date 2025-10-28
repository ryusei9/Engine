import bpy
from .route_types import ROUTE_PARENT_TYPE_KEY, SEGMENT_MODE_KEY, SEGMENT_TIME_KEY


class ROUTE_PT_tools(bpy.types.Panel):
    """ルート編集用のUI"""
    bl_label = "Route Tools"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Route'

    def draw(self, context):
        layout = self.layout

        col = layout.column(align=True)
        col.label(text="ルート作成")

        # 新規ルート作成
        row = col.row(align=True)
        row.operator("route.create", text="プレイヤー").route_type = 'PLAYER'
        row.operator("route.create", text="カメラ").route_type = 'CAMERA'

        obj = context.active_object

        # ルート親を選択しているとき
        if obj and obj.type == 'EMPTY' and ROUTE_PARENT_TYPE_KEY in obj:
            col.separator()
            col.label(text="点の追加 / 線更新")
            col.operator("route.add_point", text="点を追加（線接続）")
            col.operator("route.refresh_segments", text="線を手動更新")

            col.separator()
            col.label(text="プレビュー")
            row = col.row(align=True)
            row.operator("route.preview_modal", text="再生").action = 'PLAY'
            row.operator("route.preview_modal", text="一時停止").action = 'PAUSE'
            row.operator("route.preview_modal", text="リセット").action = 'RESET'


        # 線オブジェクトを選んでいるとき
        if obj and obj.type == 'MESH' and obj.name.startswith("RouteSeg_"):
            col.separator()
            col.label(text=f"選択中: {obj.name}")
            col.prop(obj, f'["{SEGMENT_MODE_KEY}"]', text="線の種類")
            col.prop(obj, f'["{SEGMENT_TIME_KEY}"]', text="移動時間(秒)")
            col.operator("route.set_segment_props", text="適用")

        if obj and obj.type == 'CURVE':
            col.separator()
            col.label(text="ルート編集")
            col.operator("route.add_curve_point", text="新規ポイント追加")

classes_ui = (ROUTE_PT_tools,)
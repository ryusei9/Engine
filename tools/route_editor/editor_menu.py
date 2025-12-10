import bpy

from .operators_route import (
    classes_route,
    ROUTE_OT_create_route, ROUTE_OT_add_point, ROUTE_OT_set_segment_props, ROUTE_OT_refresh_segments,
)
from .playback import ROUTE_OT_preview_modal
from common_export.export_scene import MYADDON_OT_export_scene

# トップバーの拡張メニュー
class TOPBAR_MT_route_editor_menu(bpy.types.Menu):
    bl_label = "RouteEditor"
    bl_idname = "TOPBAR_MT_route_editor_menu"
    bl_description = "拡張メニュー by Ryusei Satou"

    def draw(self, context):
        layout = self.layout
        layout.operator(ROUTE_OT_create_route.bl_idname, text="ルート新規（プレイヤー）")
        try:
            route_type = 'PLAYER'
        except AttributeError:
            # プロパティがまだ用意されていない場合は無視（安全フォールバック）
            pass
        layout.operator(ROUTE_OT_create_route.bl_idname, text="ルート新規（カメラ）")
        try:
            route_type = 'CAMERA'
        except AttributeError:
            pass
        layout.operator(ROUTE_OT_add_point.bl_idname, text="点を追加（線接続）")
        layout.operator(ROUTE_OT_refresh_segments.bl_idname, text="線更新")
        layout.operator(ROUTE_OT_set_segment_props.bl_idname, text="線のモード/時間 設定")
        layout.separator()
        #layout.operator(ROUTE_OT_preview_modal.bl_idname, text="プレビュー再生").action = 'PLAY'
        #layout.operator(ROUTE_OT_preview_modal.bl_idname, text="プレビュー一時停止").action = 'PAUSE'
        #layout.operator(ROUTE_OT_preview_modal.bl_idname, text="プレビューリセット").action = 'RESET'
        layout.separator()
        layout.operator(MYADDON_OT_export_scene.bl_idname, text="シーンを出力(JSON)")

    def submenu(self, context):
        self.layout.menu(TOPBAR_MT_route_editor_menu.bl_idname)
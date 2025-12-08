import bpy
from .operators_route import classes_route
from .panel_route import classes_ui
#from .playback import ROUTE_OT_preview_modal
from common_export.export_scene import MYADDON_OT_export_scene
from .editor_menu import TOPBAR_MT_route_editor_menu
from .route_types import SEGMENT_MODE_KEY, SEGMENT_TIME_KEY
from .operator_add_point import ROUTE_OT_add_curve_point


bl_info = {
"name": "2D Shooter Route Tools",
"author": "Peru + ChatGPT",
"version": (1, 0, 0),
"blender": (3, 0, 0),
"location": "Topbar > MyMenu / 3Dビューのサイドバー",
"description": "2Dシューター用のプレイヤー／カメラ進行ルートの作成、可視化、JSON出力",
"category": "Object",
}

# Blenderに登録するクラスリスト
classes = (
    *classes_route,
    *classes_ui,
    #ROUTE_OT_preview_modal,
    MYADDON_OT_export_scene,
    TOPBAR_MT_route_editor_menu,
    ROUTE_OT_add_curve_point,
)

# アドオン有効化時コールバック
def register():
    for cls in classes:
        bpy.utils.register_class(cls)

    # トップバーにメニューを追加
    #def draw_menu(self, context):
        #self.layout.operator("myaddon.myaddon_ot_export_scene", text="Export Scene")
    #bpy.types.TOPBAR_MT_editor_menus.append(draw_menu)

    def draw_route_menu(self, context):
        self.layout.menu("TOPBAR_MT_route_editor_menu")

    bpy.types.TOPBAR_MT_editor_menus.append(draw_route_menu)

    print("ルートエディタが有効化されました")

# アドオン無効化時コールバック
def unregister():
    # トップバーからメニューを削除
    def draw_route_menu(self, context):
        self.layout.menu(TOPBAR_MT_route_editor_menu.bl_idname)
    try:
        bpy.types.TOPBAR_MT_editor_menus.remove(draw_route_menu)
    except Exception:
        pass

    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)

    print("ルートエディタが無効化されました")

# テスト実行用
if __name__ == "__main__":
    register()
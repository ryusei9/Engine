import bpy

#モジュールのインポート
from .stretch_vertex import MYADDON_OT_stretch_vertex
from .create_ico_sphere import MYADDON_OT_create_ico_sphere
from .add_file_name import MYADDON_OT_add_file_name
from .file_name import OBJECT_PT_file_name
from .drawCollider import DrawCollider
from .add_collider import MYADDON_OT_add_collider
from .collider import OBJECT_PT_collider
from .export_scene import MYADDON_OT_export_scene
from .editor_menu import TOPBAR_MT_editor_menu

#ブレンダーに登録するアドオン情報
bl_info = {
    "name": "レベルエディタ",
    "author": "Ryusei Satou",
    "version": (1, 0),
    "blender": (3, 3, 1),
    "location": "",
    "description": "レベルエディタ",
    "warning": "",
    "wiki_url": "",
    "tracker_url":"",
    "category": "Object",
}

#Blenderに登録するクラスリスト
classes = (
    MYADDON_OT_stretch_vertex,
    MYADDON_OT_create_ico_sphere,
    MYADDON_OT_export_scene,
    TOPBAR_MT_editor_menu,
    MYADDON_OT_add_file_name,
    OBJECT_PT_file_name,
    MYADDON_OT_add_collider,
    OBJECT_PT_collider,
)
#メニュー項目描画
def draw_menu_manual(self, context):
    self.layout.operator("wm.url_open_preset", text="Manual", icon='HELP')

#アドオン有効化時コールバック
def register():
    #Blenderにクラスを登録
    for cls in classes:
        bpy.utils.register_class(cls)
    #メニュー項目を追加
    bpy.types.TOPBAR_MT_editor_menus.append(TOPBAR_MT_editor_menu.submenu)
    #3Dビューに描画関数を追加
    DrawCollider.handle = bpy.types.SpaceView3D.draw_handler_add(DrawCollider.draw_collider, (), 'WINDOW', 'POST_VIEW')
    print("レベルエディタが有効化されました")

#アドオン無効化時コールバック
def unregister():
    #メニュー項目を削除
    bpy.types.TOPBAR_MT_editor_menus.remove(TOPBAR_MT_editor_menu.submenu)
    #3Dビューから描画関数を削除
    bpy.types.SpaceView3D.draw_handler_remove(DrawCollider.handle, 'WINDOW')
    #Blenderからクラスを登録解除
    for cls in classes:
        bpy.utils.unregister_class(cls)
    print("レベルエディタが無効化されました")
    
#テスト実行用コード
if __name__ == "__main__":
    register()


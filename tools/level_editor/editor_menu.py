import bpy

from .stretch_vertex import MYADDON_OT_stretch_vertex
from .create_ico_sphere import MYADDON_OT_create_ico_sphere
from common_export.export_scene import MYADDON_OT_export_scene
from .spawn import PlayerSpawnCreateSymbol
from .spawn import EnemySpawnCreateSymbol


#トップバーの拡張メニュー
class TOPBAR_MT_editor_menu(bpy.types.Menu):
    #メニューのタイトル
    bl_label = "MyMenu"
    #BlenderのメニューID
    bl_idname = "TOPBAR_MT_editor_menu"
    #著作表示用の文字列
    bl_description = "拡張メニュー by Ryusei Satou" 

    #サブメニューの描画
    def draw(self, context):
        layout = self.layout
        #トップバーのエディターメニューに項目を追加
        layout.operator(MYADDON_OT_stretch_vertex.bl_idname, text = MYADDON_OT_stretch_vertex.bl_label)
        layout.operator(MYADDON_OT_create_ico_sphere.bl_idname, text = MYADDON_OT_create_ico_sphere.bl_label)
        layout.operator(MYADDON_OT_export_scene.bl_idname, text = MYADDON_OT_export_scene.bl_label)
        layout.operator(PlayerSpawnCreateSymbol.bl_idname, text = PlayerSpawnCreateSymbol.bl_label)
        layout.operator(EnemySpawnCreateSymbol.bl_idname, text = EnemySpawnCreateSymbol.bl_label)

    #既存のメニューにサブメニューを追加
    def submenu(self, context):
        layout = self.layout
        #ID指定でサブメニューを追加
        layout.menu(TOPBAR_MT_editor_menu.bl_idname)
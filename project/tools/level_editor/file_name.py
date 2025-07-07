import bpy
from .add_file_name import MYADDON_OT_add_file_name

#パネル　ファイル名
class OBJECT_PT_file_name(bpy.types.Panel):
    """オブジェクトのファイルネームパネル"""
    bl_idname = "OBJECT_PT_file_name"
    bl_label = "FileName"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

    #サブメニューの描画
    def draw(self, context):
        
        #パネルに項目を追加
        if "File_name" in context.object:
            #すでにプロパティがあれば、プロパティを表示
            self.layout.prop(context.object, '["File_name"]',text=self.bl_label)
        else:
            #プロパティがなければ、プロパティを追加
            self.layout.operator(MYADDON_OT_add_file_name.bl_idname)
        """self.layout.operator(MYADDON_OT_stretch_vertex.bl_idname, text = MYADDON_OT_stretch_vertex.bl_label)
        self.layout.operator(MYADDON_OT_create_ico_sphere.bl_idname, text = MYADDON_OT_create_ico_sphere.bl_label)
        self.layout.operator(MYADDON_OT_export_scene.bl_idname, text = MYADDON_OT_export_scene.bl_label)"""
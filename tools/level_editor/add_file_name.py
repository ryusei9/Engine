import bpy

#オペレータ　カスタムプロパティ['File_name']追加
class MYADDON_OT_add_file_name(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_add_file_name"
    bl_label = "FileName 追加"
    bl_description = "['File_name']カスタムプロパティを追加します"
    #リドゥ、アンドゥ可能オプション
    bl_options = {'REGISTER', 'UNDO'}

    #メニューを実行したときに呼ばれるコールバック関数
    def execute(self, context):
        #カスタムプロパティを追加
        bpy.context.object["File_name"] = ""

        #オペレータの命令終了を通知
        return {'FINISHED'}
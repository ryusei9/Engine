import mathutils
import bpy

#オペレータ　カスタムプロパティ['collider']追加
class MYADDON_OT_add_collider(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_add_collider"
    bl_label = "Collider 追加"
    bl_description = "['collider']カスタムプロパティを追加します"
    #リドゥ、アンドゥ可能オプション
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        #カスタムプロパティを追加
        context.object["collider"] = "Box"
        context.object["collider_center"] = mathutils.Vector((0,0,0))
        context.object["collider_size"] = mathutils.Vector((2,2,2))

        #オペレータの命令終了を通知
        return {'FINISHED'}
    
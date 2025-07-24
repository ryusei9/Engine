import bpy

#オペレータ 無効オプションを追加する
class MYADDON_OT_disabled(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_disabled"
    bl_label = "無効オプション追加オペレータ"
    bl_description = "無効フラグのカスタムプロパティを追加します"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        # 新たなカスタムプロパティ（bool型）を追加
        context.object["disabled"] = True
        return {'CANCELLED'}
    
# パネル 無効オプション
class OBJECT_PT_disabled(bpy.types.Panel):
    bl_idname = "OBJECT_PT_disabled"
    bl_label = "無効オプション"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

    def draw(self, context):
        layout = self.layout
        obj = context.object

        # チェックボックス表示 or 追加ボタン
        if "disabled" in obj:
            layout.prop(obj, '["disabled"]', text="無効フラグ")
        else:
            layout.operator(MYADDON_OT_disabled.bl_idname, text="無効オプションを追加")
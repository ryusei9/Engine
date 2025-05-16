import bpy
import math
import bpy_extras

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
#オペレータ　頂点を伸ばす
class MYADDON_OT_stretch_vertex(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_stretch_vertex"
    bl_label = "頂点を伸ばす"
    bl_description = "頂点座標を引っ張って伸ばします"
    #リドゥ、アンドゥ可能オプション
    bl_options = {'REGISTER', 'UNDO'}

    #メニューを実行したときに呼ばれるコールバック関数
    def execute(self, context):
        bpy.data.objects["Cube"].data.vertices[0].co.x += 1.0
        print("頂点を伸ばしました")

        #オペレータの命令終了を通知
        return {'FINISHED'}

#オペレータ　ICO球の生成
class MYADDON_OT_create_ico_sphere(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_create_object"
    bl_label = "ICO球を生成"
    bl_description = "ICO球を生成します"
    #リドゥ、アンドゥ可能オプション
    bl_options = {'REGISTER', 'UNDO'}

    #メニューを実行したときに呼ばれるコールバック関数
    def execute(self, context):
        bpy.ops.mesh.primitive_ico_sphere_add()
        print("ICO球を生成しました")

        #オペレータの命令終了を通知
        return {'FINISHED'}
    
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


#オペレータ　シーン出力
class MYADDON_OT_export_scene(bpy.types.Operator, bpy_extras.io_utils.ExportHelper):
    bl_idname = "myaddon.myaddon_ot_export_scene"
    bl_label = "シーンを出力"
    bl_description = "シーン情報をexportします"
    #出力するファイルの拡張子
    filename_ext = ".scene"

    #自動的に改行
    def write_and_print(self, file, str):
        print(str)

        file.write(str)
        file.write("\n")

    def parse_scene_recursive(self, file, object, level):
        """シーンを解析用再帰関数"""

        #深さ分インデントする
        indent = ' '
        for i in range(level):
            indent += '\t'

        #オブジェクト名書き込み
        self.write_and_print(file,indent + object.type)
        #ローカルトランスフォーム行列から平行移動、回転、スケーリングを抽出
        #型は Vector、Quaternion、Vector
        trans,rot,scale = object.matrix_local.decompose()
        #回転を Queaternion からオイラー角に変換                rot = rot.to_euler()
        #ラジアンから度数法に変換
        rot.x = math.degrees(rot.x)
        rot.y = math.degrees(rot.y)
        rot.z = math.degrees(rot.z)
        #トランスフォーム情報を出力
        self.write_and_print(file,indent + "T %f %f %f" % (trans.x,trans.y,trans.z))
        self.write_and_print(file,indent + "R %f %f %f" % (rot.x,rot.y,rot.z))
        self.write_and_print(file,indent + "S %f %f %f" % (scale.x,scale.y,scale.z))
        #カスタムプロパティを出力
        if "File_name" in object:
            self.write_and_print(file,indent + "N %s" % object["File_name"])
        self.write_and_print(file,indent + 'END')
        self.write_and_print(file,"")

        for child in object.children:
            self.parse_scene_recursive(file, child, level + 1)

    def export(self):
        """ファイルに出力"""

        print("シーン情報出力開始... %r" % self.filepath)

        #ファイルをテキスト形式で書き出し用にオープン
        #スコープを抜けると自動でクローズされる
        with open(self.filepath, 'wt') as file:
            #ファイルに文字列を書き込む
            file.write("SCENE\n")

            #シーン内の全オブジェクトについて
            for object in bpy.context.scene.objects:
                if(object.parent):
                    continue

                #シーン直下のオブジェクトをルートノード（深さ0）とし、再帰関数で走査
                self.parse_scene_recursive(file, object, 0)

           

    #メニューを実行したときに呼ばれるコールバック関数
    def execute(self, context):
       
        print("シーン情報をexportします")

        #ファイルに出力
        self.export()

        

        print("シーン情報をexportしました")
        self.report({'INFO'}, "シーン情報をexportしました")

        #オペレータの命令終了を通知
        return {'FINISHED'}
#トップバーの拡張メニュー
class TOPBAR_MT_editor_menu(bpy.types.Menu):
    #メニューのタイトル
    bl_label = "MyMenu"
    #BlenderのメニューID
    bl_idname = "TOPBAR_MT_editor_menu"
    #著作表示用の文字列
    bl_description = "拡張メニュー by " + bl_info["author"]

    #サブメニューの描画
    def draw(self, context):
        #トップバーのエディターメニューに項目を追加
        self.layout.operator(MYADDON_OT_stretch_vertex.bl_idname, text = MYADDON_OT_stretch_vertex.bl_label)
        self.layout.operator(MYADDON_OT_create_ico_sphere.bl_idname, text = MYADDON_OT_create_ico_sphere.bl_label)
        self.layout.operator(MYADDON_OT_export_scene.bl_idname, text = MYADDON_OT_export_scene.bl_label)
    #既存のメニューにサブメニューを追加
    def submenu(self, context):
        #ID指定でサブメニューを追加
        self.layout.menu(TOPBAR_MT_editor_menu.bl_idname)

#Blenderに登録するクラスリスト
classes = (
    MYADDON_OT_stretch_vertex,
    MYADDON_OT_create_ico_sphere,
    MYADDON_OT_export_scene,
    TOPBAR_MT_editor_menu,
    MYADDON_OT_add_file_name,
    OBJECT_PT_file_name,
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
    print("レベルエディタが有効化されました")

#アドオン無効化時コールバック
def unregister():
    #メニュー項目を削除
    bpy.types.TOPBAR_MT_editor_menus.remove(TOPBAR_MT_editor_menu.submenu)
    #Blenderからクラスを登録解除
    for cls in classes:
        bpy.utils.unregister_class(cls)
    print("レベルエディタが無効化されました")
    
#テスト実行用コード
if __name__ == "__main__":
    register()


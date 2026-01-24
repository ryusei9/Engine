import bpy
import os
import bpy.ops

# =========================
# Enemy Move Type Enum
# =========================
def enemy_move_items(self, context):
    bpy.types.Object.enemy_move = bpy.props.EnumProperty(
        name="Enemy Move",
        items=[
            ("None", "None", ""),
            ("Enemy_Wave_-Z", "Wave -Z", ""),
            ("Enemy_Wave_+Z", "Wave +Z", ""),
        ],
         default=0
    )

def register_enemy_props():
    bpy.types.Object.enemy_move = bpy.props.EnumProperty(
        name="Enemy Move",
        description="Enemy movement curve",
        items=enemy_move_items,
         default=0
    )

def unregister_enemy_props():
    del bpy.types.Object.enemy_move

#オペレータ 出現ポイントのシンボルを読み込む
class MYADDON_OT_spawn_import_symbol(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_spawn_import_symbol"
    bl_label = "出現ポイントシンボルImport"
    bl_description = "出現ポイントのシンボルをImportします"
    prototype_object_name = "PrototypePlayerSpawn"
    object_name = "PlayerSpawn"

    def load_obj(self, type):
        print("出現ポイントのシンボルをImportします")
        #重複ロード防止
        spawn_object = bpy.data.objects.get(SpawnNames.names[type][SpawnNames.PROTOTYPE])
        if spawn_object:
            return {'CANCELLED'}
        # スクリプトが配置されているディレクトリの名前を取得する
        addon_directory = os.path.dirname(__file__)
        # ディレクトリからのモデルファイルの相対パスを記述
        relative_path = SpawnNames.names[type][SpawnNames.FILENAME]
        # 合成してモデルファイルのフルパスを取得
        full_path = os.path.join(addon_directory, relative_path)

        #オブジェクトをインポート
        bpy.ops.wm.obj_import('EXEC_DEFAULT',
                              filepath = full_path,display_type = 'THUMBNAIL',
                              forward_axis = 'Z', up_axis = 'Y',)
        
        #回転を適用
        bpy.ops.object.transform_apply(location=False,
            rotation=True, scale=False,
            isolate_users=False)
        
        #アクティブなオブジェクトを取得
        object = bpy.context.active_object
        # ←ここで使う名前をselfで定義した変数に置き換える
        if type == "Player":
            prototype_name = "PlayerSpawn"
            object_name = "PlayerSpawn"
        else:
            prototype_name = "EnemySpawn"
            object_name = "EnemySpawn"

        object.name = prototype_name
        object["type"] = object_name
        bpy.context.collection.objects.unlink(object)

        return {'FINISHED'}
    
    def execute(self, context):
        #ENEMYオブジェクト読み込み
        self.load_obj("Enemy")
        #PLAYERオブジェクト読み込み
        self.load_obj("Player")

        return {'FINISHED'}
    
#オペレータ　出現ポイントのシンボルを作成・配置する
class MYADDON_OT_spawn_create_symbol(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_spawn_create_symbol"
    bl_label = "出現ポイントシンボルを配置"
    bl_description = "出現ポイントのシンボルを配置します"
    bl_options = {'REGISTER', 'UNDO'}

    type = bpy.props.StringProperty(name = "type",default = "Player")

    def execute(self, context):
        #プロトタイプオブジェクトを取得
        spawn_object = bpy.data.objects.get(SpawnNames.names[self.type][SpawnNames.PROTOTYPE])

        #まだ読み込んでいない場合
        if spawn_object is None:
            #読み込みオペレータを実行
            bpy.ops.myaddon.myaddon_ot_spawn_import_symbol('EXEC_DEFAULT')
            #再度取得
            spawn_object = bpy.data.objects.get(SpawnNames.names[self.type][SpawnNames.PROTOTYPE])

        print("出現ポイントのシンボルを作成します")

        #blenderでの選択を解除
        bpy.ops.object.select_all(action='DESELECT')

        #複製元の非表示オブジェクトを複製する
        spawn_object = spawn_object.copy()

        #複製したオブジェクトを現在のシーンにリンク
        bpy.context.collection.objects.link(spawn_object)

        #オブジェクト名を変更
        SpawnNames.names[self.type][SpawnNames.INSTANCE]

        return {'FINISHED'}

class SpawnNames():
    #インデックス
    PROTOTYPE = 0   # プロトタイプオブジェクト
    INSTANCE = 1    # インスタンスオブジェクト
    FILENAME = 2    # ファイル名

    names = {}

    names["Enemy"] = ("EnemySpawn", "EnemySpawn", "enemy/enemy.obj")
    names["Player"] = ("PlayerSpawn", "PlayerSpawn", "player/player.obj")

class PlayerSpawnCreateSymbol(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_spawn_create_player_symbol"
    bl_label = "プレイヤー出現ポイントシンボルの作成"
    bl_description = "プレイヤー出現ポイントのシンボルを作成します"

    def execute(self, context):
        #プレイヤー出現ポイントのシンボルを作成
        bpy.ops.myaddon.myaddon_ot_spawn_create_symbol('EXEC_DEFAULT', type="Player")
        return {'FINISHED'}
    
class EnemySpawnCreateSymbol(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_spawn_create_enemy_symbol"
    bl_label = "敵出現ポイントシンボルの作成"
    bl_description = "敵出現ポイントのシンボルを作成します"

    def execute(self, context):
        #敵出現ポイントのシンボルを作成
        bpy.ops.myaddon.myaddon_ot_spawn_create_symbol('EXEC_DEFAULT', type="Enemy")
        return {'FINISHED'}
    
class MYADDON_PT_enemy_spawn_panel(bpy.types.Panel):
    bl_label = "Enemy Spawn"
    bl_idname = "MYADDON_PT_enemy_spawn_panel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Spawn'

    def draw(self, context):
        layout = self.layout
        obj = context.active_object

        if not obj:
            return

        # EnemySpawn のみ表示
        if obj.get("type") != "EnemySpawn":
            layout.label(text="Select EnemySpawn")
            return

        layout.label(text="Enemy Movement")
        layout.prop(obj, "enemy_move", text="")
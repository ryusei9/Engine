import bpy
import bpy_extras
import math
import json

# オペレータ　シーン出力
class MYADDON_OT_export_scene(bpy.types.Operator, bpy_extras.io_utils.ExportHelper):
    bl_idname = "myaddon.myaddon_ot_export_scene"
    bl_label = "シーンを出力"
    bl_description = "シーン情報をexportします"
    filename_ext = ".json"

    # 自動的に改行
    def write_and_print(self, file, str):
        print(str)
        file.write(str)
        file.write("\n")

    def parse_scene_recursive(self, file, object, level):
        """シーンを解析用再帰関数（旧形式：テキスト出力）"""
        indent = ' '
        for i in range(level):
            indent += '\t'

        self.write_and_print(file, indent + object.type)
        trans, rot, scale = object.matrix_local.decompose()
        rot = rot.to_euler()
        rot.x, rot.y, rot.z = map(math.degrees, (rot.x, rot.y, rot.z))

        self.write_and_print(file, indent + f"T {trans.x:.6f} {trans.y:.6f} {trans.z:.6f}")
        self.write_and_print(file, indent + f"R {rot.x:.6f} {rot.y:.6f} {rot.z:.6f}")
        self.write_and_print(file, indent + f"S {scale.x:.6f} {scale.y:.6f} {scale.z:.6f}")

        if "disabled" in object:
            self.write_and_print(file, indent + f"D {object['disabled']}")
        if "File_name" in object:
            self.write_and_print(file, indent + f"N {object['File_name']}")
        if "collider" in object:
            self.write_and_print(file, indent + f"C {object['collider']}")
            cc = object["collider_center"]
            cs = object["collider_size"]
            self.write_and_print(file, indent + f"CC {cc[0]:.6f} {cc[1]:.6f} {cc[2]:.6f}")
            self.write_and_print(file, indent + f"CS {cs[0]:.6f} {cs[1]:.6f} {cs[2]:.6f}")

        self.write_and_print(file, indent + 'END')
        self.write_and_print(file, "")

        for child in object.children:
            self.parse_scene_recursive(file, child, level + 1)

    def export(self):
        """ファイルに出力（旧形式：テキスト）"""
        print("シーン情報出力開始... %r" % self.filepath)
        with open(self.filepath, 'wt') as file:
            file.write("SCENE\n")
            for object in bpy.context.scene.objects:
                if object.parent:
                    continue
                self.parse_scene_recursive(file, object, 0)

    def parse_scene_recursive_json(self, data_parent, object, level):
        """JSON形式出力用再帰関数"""
        json_object = dict()
        json_object["type"] = getattr(object, "type", "UNKNOWN")
        json_object["name"] = object.name

        # --- トランスフォーム情報 ---
        trans, rot, scale = object.matrix_local.decompose()
        rot = rot.to_euler()
        rot.x, rot.y, rot.z = map(math.degrees, (rot.x, rot.y, rot.z))
        transform = {
            "translation": [trans.x, trans.y, trans.z],
            "rotation": [rot.x, rot.y, rot.z],
            "scale": [scale.x, scale.y, scale.z],
        }
        json_object["transform"] = transform

        # --- カスタムプロパティ ---
        for key, value in object.items():
            if key not in "_RNA_UI":
                json_object[key] = value

         # --- Curveデータ対応（制御点と時間の出力） ---
        if object.type == 'CURVE':
            curve_data = object.data
            json_object["curve"] = {"splines": []}

            # ★ 制御点時間を取得（なければ None）
            point_times = curve_data.get("times", [])
            
            for spline in curve_data.splines:
                spline_data = {"type": spline.type, "points": []}

                if spline.type == 'BEZIER':
                    for i, p in enumerate(spline.bezier_points):
                        point_data = {
                            "co": [p.co.x, p.co.y, p.co.z],
                            "handle_left": [p.handle_left.x, p.handle_left.y, p.handle_left.z],
                            "handle_right": [p.handle_right.x, p.handle_right.y, p.handle_right.z],
                        }

                        # ★ インデックスと照合して time を出力
                        if point_times and i < len(point_times):
                            point_data["time"] = point_times[i]

                        spline_data["points"].append(point_data)

                else:
                    for i, p in enumerate(spline.points):
                        point_data = {
                            "co": [p.co.x, p.co.y, p.co.z, p.co.w],
                        }

                        if point_times and i < len(point_times):
                            point_data["time"] = point_times[i]

                        spline_data["points"].append(point_data)

                json_object["curve"]["splines"].append(spline_data)

        # --- 子オブジェクト ---
        if len(object.children) > 0:
            json_object["children"] = []
            for child in object.children:
                self.parse_scene_recursive_json(json_object["children"], child, level + 1)

        data_parent.append(json_object)

    def export_json(self):
        """JSON形式でファイルに出力"""
        json_object_root = {"name": "scene", "objects": []}

        for object in bpy.context.scene.objects:
            if object.parent:
                continue
            self.parse_scene_recursive_json(json_object_root["objects"], object, 0)

        json_text = json.dumps(json_object_root, ensure_ascii=False, indent=4)
        print(json_text)

        with open(self.filepath, "wt", encoding="utf-8") as file:
            file.write(json_text)

    # 実行処理
    def execute(self, context):
        print("シーン情報をexportします")
        self.export_json()
        print("シーン情報をexportしました")
        self.report({'INFO'}, "シーン情報をexportしました")
        return {'FINISHED'}

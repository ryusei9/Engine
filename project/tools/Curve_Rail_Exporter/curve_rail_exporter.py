bl_info = {
    "name": "Curve Rail Exporter",
    "author": "Ryusei Sato + ChatGPT",
    "version": (1, 0, 0),
    "blender": (3, 0, 0),
    "location": "View3D > Sidebar (N) > Curve Rail",
    "description": "Curve にレールフラグを付け、制御点を JSON 出力します",
    "category": "Object",
}

import bpy
import json
import math
import os
from bpy_extras.io_utils import ExportHelper
from mathutils import Vector

# ----------------------------
# ユーティリティ
# ----------------------------
def vec_to_list(v: Vector):
    # Vector を (x,y,z) の list にする（ワールド座標）
    return [float(v.x), float(v.y), float(v.z)]

def get_curve_control_points_world(obj: bpy.types.Object):
    """Curveオブジェクトのすべてのスプラインの制御点をワールド座標で取得する。
    戻り値は spline 単位の配列（各スプラインは dict）"""
    data = obj.data
    result = []
    for si, spline in enumerate(data.splines):
        sdict = {"index": si, "type": spline.type, "points": []}
        if spline.type == 'BEZIER':
            for pi, bp in enumerate(spline.bezier_points):
                co_world = obj.matrix_world @ bp.co
                handle_left_world = obj.matrix_world @ bp.handle_left
                handle_right_world = obj.matrix_world @ bp.handle_right
                sdict["points"].append({
                    "index": pi,
                    "co": vec_to_list(co_world),
                    "handle_left": vec_to_list(handle_left_world),
                    "handle_right": vec_to_list(handle_right_world),
                    # local coordinates useful? include if needed
                    "co_local": [float(bp.co.x), float(bp.co.y), float(bp.co.z)]
                })
        else:
            # POLY/NURBS はこちら（spline.points は 4D: x,y,z,w）
            for pi, pt in enumerate(spline.points):
                # pt.co は Vector of length 4
                co = Vector((pt.co[0], pt.co[1], pt.co[2]))
                co_world = obj.matrix_world @ co
                sdict["points"].append({
                    "index": pi,
                    "co": vec_to_list(co_world),
                    "co_local": [float(pt.co[0]), float(pt.co[1]), float(pt.co[2])]
                })
        result.append(sdict)
    return result

# ----------------------------
# Export Operator
# ----------------------------
class CURVE_RAIL_OT_export_json(bpy.types.Operator, ExportHelper):
    """カーブ（レール）から制御点を JSON 出力"""
    bl_idname = "curve_rail.export_json"
    bl_label = "Export Curve Rails to JSON"
    filename_ext = ".json"

    # ExportHelper が使うプロパティ
    filter_glob = bpy.props.StringProperty(default="*.json", options={'HIDDEN'})

    # 出力対象の選択モード
    export_mode = bpy.props.EnumProperty(
        name="Export Mode",
        items=[
            ('SELECTED', "Selected Curves", "選択中のカーブオブジェクトを出力"),
            ('RAILS', "All Rails", "is_rail=True のすべてのカーブを出力"),
        ],
        default='SELECTED'
    )

    include_empty_objects = bpy.props.BoolProperty(
        name="Include non-curve objects",
        default=False,
        description="カーブでないオブジェクトを誤って選択していた場合に除外する"
    )

    def execute(self, context):
        # 収集対象
        objs = []
        if self.export_mode == 'SELECTED':
            for o in context.selected_objects:
                if o.type == 'CURVE':
                    objs.append(o)
                elif not self.include_empty_objects:
                    # skip non-curve
                    continue
        else:  # RAILS
            for o in context.scene.objects:
                if o.type == 'CURVE' and o.get("is_rail", False):
                    objs.append(o)

        if not objs:
            self.report({'WARNING'}, "対象の Curve オブジェクトが見つかりませんでした")
            return {'CANCELLED'}

        root = {"curves": []}
        for o in objs:
            entry = {
                "name": o.name,
                "is_rail": bool(o.get("is_rail", False)),
                "location": vec_to_list(o.matrix_world.translation),
                "splines": get_curve_control_points_world(o),
            }
            root["curves"].append(entry)

        # ファイルパス
        filepath = bpy.path.abspath(self.filepath)
        # 相対パス処理（デフォルトは blend ファイルと同じフォルダ）
        if not os.path.isabs(filepath):
            blend_dir = os.path.dirname(bpy.data.filepath) or os.path.expanduser("~")
            filepath = os.path.join(blend_dir, filepath)

        # 書き出し（インデントあり）
        try:
            with open(filepath, "w", encoding="utf-8") as f:
                json.dump(root, f, ensure_ascii=False, indent=4)
            self.report({'INFO'}, f"Exported {len(root['curves'])} curve(s) to {filepath}")
        except Exception as e:
            self.report({'ERROR'}, f"Failed to write file: {e}")
            return {'CANCELLED'}

        return {'FINISHED'}

# ----------------------------
# Panel UI
# ----------------------------
class CURVE_RAIL_PT_panel(bpy.types.Panel):
    bl_label = "Curve Rail"
    bl_idname = "CURVE_RAIL_PT_panel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Curve Rail'

    def draw(self, context):
        layout = self.layout
        obj = context.active_object

        col = layout.column(align=True)
        col.label(text="Rail Flag")
        if obj and obj.type == 'CURVE':
            # カスタムプロパティを直接編集する
            # ない場合は作成される
            if "is_rail" not in obj:
                obj["is_rail"] = False
            col.prop(obj, '["is_rail"]', text="is_rail")

            col.separator()
            col.label(text="Preview control points")
            col.operator("curve_rail.show_points_in_console", text="Print Points to Console")
        else:
            col.label(text="Select a Curve object to set rail flag", icon='INFO')

        col.separator()
        col.label(text="Export")
        row = col.row(align=True)
        row.operator(CURVE_RAIL_OT_export_json.bl_idname, text="Export Selected").export_mode = 'SELECTED'
        row.operator(CURVE_RAIL_OT_export_json.bl_idname, text="Export Rails").export_mode = 'RAILS'

# ----------------------------
# Helper operator: プレビュー（コンソール出力）
# ----------------------------
class CURVE_RAIL_OT_print_points(bpy.types.Operator):
    bl_idname = "curve_rail.show_points_in_console"
    bl_label = "Print curve control points"

    def execute(self, context):
        obj = context.active_object
        if not obj or obj.type != 'CURVE':
            self.report({'WARNING'}, "Curve オブジェクトを選択してください")
            return {'CANCELLED'}
        splines = get_curve_control_points_world(obj)
        print("Curve:", obj.name)
        for s in splines:
            print(" Spline", s["index"], "type", s["type"])
            for p in s["points"]:
                print("  pt", p["index"], "co", p["co"])
        self.report({'INFO'}, "制御点をコンソールに出力しました")
        return {'FINISHED'}

# ----------------------------
# registration
# ----------------------------
classes = (
    CURVE_RAIL_OT_export_json,
    CURVE_RAIL_PT_panel,
    CURVE_RAIL_OT_print_points,
)

def register():
    for c in classes:
        bpy.utils.register_class(c)

def unregister():
    for c in reversed(classes):
        bpy.utils.unregister_class(c)

# スクリプト単体実行用
if __name__ == "__main__":
    register()

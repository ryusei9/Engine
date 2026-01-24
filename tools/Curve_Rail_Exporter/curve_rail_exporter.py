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

# =========================
# Operator: Curve Role 初期化
# =========================
class CURVE_OT_init_role(bpy.types.Operator):
    bl_idname = "curve.init_role"
    bl_label = "Initialize Curve Role"
    bl_description = "Initialize curve role (CAMERA / ENEMY)"

    def execute(self, context):
        obj = context.object
        if not obj or obj.type != 'CURVE':
            return {'CANCELLED'}

        if "curve_role" not in obj:
            obj["curve_role"] = "CAMERA"

        return {'FINISHED'}

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

# ==================================================
# Enum Property
# ==================================================

def register_props():
    bpy.types.Object.curve_role = bpy.props.EnumProperty(
        name="Curve Role",
        description="Role of this curve",
        items=[
            ("None", "None", ""),
            ("Enemy_Wave_-Z", "Wave -Z", ""),
            ("Enemy_Wave_+Z", "Wave +Z", ""),
        ],
        default='None'
    )

def unregister_props():
    del bpy.types.Object.curve_role

# ----------------------------
# Export Operator
# ----------------------------
class CURVE_RAIL_OT_export_json(bpy.types.Operator):
    bl_idname = "curve.export_json"
    bl_label = "Export Curve JSON"

    export_mode = bpy.props.EnumProperty(
        name="Export Mode",
        items=[
            ('ALL', "All Curves", "Export all curves"),
            ('SELECTED', "Selected Only", "Export only selected curves"),
        ],
        default='ALL'
    )

    def execute(self, context):
        print("Export Mode:", self.export_mode)
        objs = []
        if self.export_mode == 'SELECTED':
            for o in context.selected_objects:
                if o.type == 'CURVE':
                    if "is_rail" not in o:
                        o["is_rail"] = False
                    objs.append(o)
                
            print("export_mode =", self.export_mode)
        else:  # ALL
            for o in context.scene.objects:
                if o.type == 'CURVE':
                    objs.append(o)

        if not objs:
            self.report({'WARNING'}, "対象の Curve オブジェクトが見つかりませんでした")
            return {'CANCELLED'}

        root = {"curves": []}
        for o in objs:
            entry = {
                "name": o.name,
                "curve_role": o.curve_role,
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

        if not obj or obj.type != 'CURVE':
            layout.label(text="Select a curve")
            return

        col = layout.column()

        # ---- Curve Role ----
        col.label(text="Curve Role")

        col.prop(obj, "curve_role", text="Role")

        col.separator()
        #col.label(text="Rail Flag")
        #if obj and obj.type == 'CURVE':
            #if "is_rail" in obj:
                #col.prop(obj, '["is_rail"]', text="Rail")
            #else:
                #col.label(text="(Rail 未設定 - Export時に自動設定)", icon="INFO")
        #row = col.row(align=True)

        #op = row.operator(CURVE_RAIL_OT_export_json.bl_idname, text="Export Selected")
        #op.export_mode = 'SELECTED'
        #op2 = row.operator(CURVE_RAIL_OT_export_json.bl_idname, text="Export Rails")
        #op2.export_mode = 'ALL'
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
# Property Group: 制御点の時間情報
# ----------------------------
class RailPointTime(bpy.types.PropertyGroup):
    value = bpy.props.FloatProperty(
        name="Time",
        default=1.0,
        min=0.0,
        description="この制御点に到達するまでの時間"
    )
    
# ----------------------------
# Operator: times 配列の初期化・修正
# ----------------------------
class CURVE_OT_fix_time_array(bpy.types.Operator):
    bl_idname = "curve.fix_time_array"
    bl_label = "Initialize / Fix Time List"

    def execute(self, context):
        obj = context.object

        if not obj or obj.type != 'CURVE':
            self.report({'WARNING'}, "Curve not selected")
            return {'CANCELLED'}

        curve = obj.data
        points = curve.splines[0].bezier_points
        count = len(points)

        # ないなら作る
        if "times" not in curve:
            curve["times"] = [1.0] * count
        else:
            times = list(curve["times"])

            if len(times) < count:
                times.extend([1.0] * (count - len(times)))
            elif len(times) > count:
                times = times[:count]

            curve["times"] = times

        self.report({'INFO'}, "Time data updated.")
        return {'FINISHED'}

# ----------------------------
# Curve Time Adjustment Panel
# ----------------------------

class CURVE_PT_time_adjust_panel(bpy.types.Panel):
    bl_label = "Curve Time Controls"
    bl_idname = "CURVE_PT_time_adjust_panel"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "data"

    def draw(self, context):
        layout = self.layout
        obj = context.object

        if not obj or obj.type != 'CURVE':
            layout.label(text="Select a Curve")
            return

        curve = obj.data

        # ボタンでデータ作成
        layout.operator("curve.fix_time_array")

        if "times" not in curve:
            layout.label(text="No time data found.")
            return

        layout.label(text="Control Point Times:")

        times = list(curve["times"])

        for i, t in enumerate(times):
            layout.prop(curve, '["times"]', index=i, text=f"Point {i}")


# ----------------------------
# registration
# ----------------------------
classes = (
    CURVE_OT_init_role,
    CURVE_RAIL_OT_export_json,
    CURVE_RAIL_PT_panel,
    CURVE_RAIL_OT_print_points,
    CURVE_PT_time_adjust_panel,
    CURVE_OT_fix_time_array,
)

def register():
    register_props()
    bpy.utils.register_class(RailPointTime)
    bpy.types.Curve.time_settings = bpy.props.CollectionProperty(type=RailPointTime)
    for c in classes:
        bpy.utils.register_class(c)

def unregister():
    del bpy.types.Curve.time_settings
    bpy.utils.unregister_class(RailPointTime)
    for c in reversed(classes):
        bpy.utils.unregister_class(c)
    unregister_props()

# スクリプト単体実行用
if __name__ == "__main__":
    register()

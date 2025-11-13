import bpy
from bpy.props import EnumProperty, FloatProperty, BoolProperty


ROUTE_PARENT_TYPE_KEY = "route_type" # 親オブジェクト（Empty）に付与
ROUTE_TYPE_PLAYER = "PLAYER"
ROUTE_TYPE_CAMERA = "CAMERA"


SEGMENT_MODE_KEY = "segment_mode" # セグメント(線オブジェクト)に付与
SEGMENT_TIME_KEY = "segment_time" # セグメント(線オブジェクト)に付与（秒）


SEGMENT_MODE_ITEMS = [
("LINE", "直線", "直線で補間"),
("CURVE", "曲線", "Catmull-Romスプライン的に補間"),
]


DEFAULT_SEGMENT_TIME = 1.0


# ルート親Empty作成ユーティリティ


def make_route_parent(name: str, route_type: str) -> bpy.types.Object:
 parent = bpy.data.objects.new(name, None)
 parent.empty_display_type = 'PLAIN_AXES'
 parent[ROUTE_PARENT_TYPE_KEY] = route_type
 bpy.context.scene.collection.objects.link(parent)
 return parent




def make_point(name: str, parent: bpy.types.Object, location) -> bpy.types.Object:
 pt = bpy.data.objects.new(name, None)
 pt.empty_display_type = 'CUBE'
 pt.empty_display_size = 0.2
 pt.parent = parent
 pt.location = location
 bpy.context.scene.collection.objects.link(pt)
 return pt




def make_segment_obj(name: str, p0: bpy.types.Object, p1: bpy.types.Object) -> bpy.types.Object:
# 2頂点1エッジのメッシュオブジェクトを生成
 import bmesh
 mesh = bpy.data.meshes.new(name + "_mesh")
 bm = bmesh.new()
 v0 = bm.verts.new(p0.location)
 v1 = bm.verts.new(p1.location)
 bm.edges.new((v0, v1))
 bm.to_mesh(mesh)
 bm.free()
 seg = bpy.data.objects.new(name, mesh)
 seg.display_type = 'WIRE'
 bpy.context.scene.collection.objects.link(seg)
 # セグメントは親Emptyの子に
 seg.parent = p0.parent
 # カスタムプロパティ初期値
 seg[SEGMENT_MODE_KEY] = 'LINE'
 seg[SEGMENT_TIME_KEY] = DEFAULT_SEGMENT_TIME
 return seg
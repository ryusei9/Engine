#pragma once
#include "ModelCommon.h"
#include "ModelData.h"
#include "Matrix4x4.h"
#include <vector>
#include "string"
#include "fstream"
#include "Material.h"

namespace {
	// OBJファイル形式の定数
	constexpr int32_t kFaceVertexCount = 3;      // 三角形の頂点数
	constexpr int32_t kFaceElementCount = 3;     // pos/uv/normal の要素数
	constexpr int32_t kObjIndexOffset = 1;       // OBJファイルの1始まりインデックスのオフセット

	// 座標変換の定数
	constexpr float kCoordinateFlipScale = -1.0f;  // 右手系→左手系の反転係数
	constexpr float kDefaultPositionW = 1.0f;      // 位置ベクトルのw成分

	// マテリアルのデフォルト値
	constexpr float kDefaultMaterialColorR = 1.0f;
	constexpr float kDefaultMaterialColorG = 1.0f;
	constexpr float kDefaultMaterialColorB = 1.0f;
	constexpr float kDefaultMaterialColorA = 1.0f;
	constexpr bool kDefaultLightingEnabled = false;

	// 識別子文字列
	constexpr const char* kObjIdentifierVertex = "v";
	constexpr const char* kObjIdentifierTexCoord = "vt";
	constexpr const char* kObjIdentifierNormal = "vn";
	constexpr const char* kObjIdentifierFace = "f";
	constexpr const char* kObjIdentifierMaterialLib = "mtllib";
	constexpr const char* kMtlIdentifierTexture = "map_Kd";
	constexpr char kFaceDelimiter = '/';
}

/// <summary>
/// 3Dモデル
/// </summary>
class Model
{
public:
	// 初期化
	void Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename);

	// 描画
	void Draw();

	// .mtlファイルの読み込み
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	// .objファイルの読み取り
	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

	// ゲッター
	const ModelData& GetModelData() const { return modelData_; }

private:
	// BufferResourceの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, size_t sizeInBytes);
	
	// 頂点データの作成
	void CreateVertexData();
	
	// マテリアルデータの作成
	void CreateMaterialData();

	// メンバ変数
	ModelCommon* modelCommon_ = nullptr;

	// Objファイルのデータ
	ModelData modelData_;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;

	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData_ = nullptr;
	Material* materialData_ = nullptr;

	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	// OBJパース用ヘルパー関数
	static Vector4 ParseVertexPosition(std::istringstream& stream);
	static Vector2 ParseTexCoord(std::istringstream& stream);
	static Vector3 ParseNormal(std::istringstream& stream);
	static void ParseFace(
		std::istringstream& stream,
		const std::vector<Vector4>& positions,
		const std::vector<Vector2>& texcoords,
		const std::vector<Vector3>& normals,
		ModelData& modelData);
	static void ParseVertexIndices(const std::string& vertexDefinition, uint32_t* outIndices);
};

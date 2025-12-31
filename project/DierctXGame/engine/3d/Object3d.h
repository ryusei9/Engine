#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <vector>
#include "string"
#include "fstream"
#include "Matrix4x4.h"
#include "worldTransform.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "Model.h"
#include "Camera.h"
#include <numbers>
#include <MaterialData.h>
#include <Material.h>
#include <VertexData.h>
#include <ModelData.h>

// 前方宣言
class Object3dCommon;

// Object3d用の定数
namespace Object3dConstants {
	// デフォルトのリソースディレクトリ
	constexpr const char* kDefaultResourceDirectory = "resources";
	constexpr const char* kDefaultSkyboxFilePath = "resources/skybox.dds";
	
	// 球体の分割数
	constexpr uint32_t kSphereSubdivision = 32;
	constexpr uint32_t kSphereVerticesPerQuad = 6;
	
	// ライトのデフォルト値
	constexpr float kDefaultLightIntensity = 3.0f;
	constexpr float kDefaultPointLightIntensity = 1.0f;
	constexpr float kDefaultPointLightRadius = 10.0f;
	constexpr float kDefaultPointLightDecay = 1.0f;
	constexpr float kDefaultSpotLightIntensity = 4.0f;
	constexpr float kDefaultSpotLightDistance = 7.0f;
	constexpr float kDefaultSpotLightDecay = 2.0f;
	
	// マテリアルのデフォルト値
	constexpr float kDefaultMaterialColorR = 1.0f;
	constexpr float kDefaultMaterialColorG = 1.0f;
	constexpr float kDefaultMaterialColorB = 1.0f;
	constexpr float kDefaultMaterialColorA = 1.0f;
	constexpr float kDefaultShininess = 50.0f;
	constexpr float kDefaultEnvironmentCoefficient = 0.0f;
	constexpr bool kDefaultLightingEnabled = true;
	
	// ポジションのデフォルト値
	constexpr float kDefaultLightPositionY = 2.0f;
	constexpr float kDefaultDirectionalLightDirectionY = -1.0f;
	
	// スポットライトの角度
	constexpr float kSpotLightAngleDivisor = 3.0f;
	constexpr float kSpotLightFalloffAngleDivisor = 6.0f;
	
	// OBJ形式の定数
	constexpr int32_t kFaceVertexCount = 3;
	constexpr int32_t kFaceElementCount = 3;
	constexpr int32_t kObjIndexOffset = 1;
	constexpr float kCoordinateFlipScale = -1.0f;
	constexpr float kDefaultPositionW = 1.0f;
	
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
/// 3Dオブジェクト
/// </summary>
class Object3d
{
public:
	// 座標変換行列データ
	struct WorldTransformationMatrix {
		Matrix4x4 wvp;
		Matrix4x4 world;
		Matrix4x4 worldInverseTranspose;
	};

	// ディレクショナルライトデータ
	struct DirectionalLight {
		Vector4 color; // ライトの色
		Vector3 direction; // ライトの向き
		float intensity; // 輝度
	};

	struct CameraForGPU {
		Vector3 worldPosition; // カメラのワールド座標
	};

	// ポイントライトデータ
	struct PointLight {
		Vector4 color; // ライトの色
		Vector3 position; // ライトの位置
		float intensity; // 輝度
		float radius; // ライトの届く最大距離
		float decay; // 減衰率
	};

	// スポットライトデータ
	struct SpotLight {
		Vector4 color; // ライトの色
		Vector3 position; // ライトの位置
		float intensity; // 輝度
		Vector3 direction; // ライトの向き
		float cosAngle; // スポットライトの角度
		float cosFalloffStart; // スポットライトの開始角度の余弦値
		float distance; // ライトの届く最大距離
		float decay; // 減衰率
		float padding[2]; // パディング
	};

	// 初期化
	void Initialize(const std::string& fileName);

	// 更新
	void Update();

	// 描画
	void Draw();

	// .mtlファイルの読み込み
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	// .objファイルの読み取り
	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

	// ImGui描画
	void DrawImGui();

	// ゲッター
	const Vector3& GetScale() const { return worldTransform.scale_; }
	const Vector3& GetRotate() const { return worldTransform.rotate_; }
	const Vector3& GetTranslate() const { return worldTransform.translate_; }

	// セッター
	void SetModel(Model* model) { model_ = model; }
	void SetModel(const std::string& filePath);
	void SetScale(const Vector3& scale) { worldTransform.scale_ = scale; }
	void SetRotate(const Vector3& rotate) { worldTransform.rotate_ = rotate; }
	void SetTranslate(const Vector3& translate) { worldTransform.translate_ = translate; }
	void SetWorldTransform(const WorldTransform& worldTransform) { this->worldTransform = worldTransform; }
	void SetCamera(Camera* camera) { camera_ = camera; }
	void SetSkyboxFilePath(std::string filePath);
	void SetPointLight(float intensity) { pointLightData_->intensity = intensity; }
	void SetSpotLight(float intensity) { spotLightData_->intensity = intensity; }
	void SetDirectionalLight(float intensity) { directionalLightData_->intensity = intensity; }
	void SetMaterialColor(const Vector4& color) { materialData_->color = color; }

private:
	// BufferResourceの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, size_t sizeInBytes);
	
	// リソース作成
	void CreateVertexData();
	void CreateMaterialData();
	void CreateDirectionalLightData();
	void CreateCameraResource();
	void CreatePointLightResource();
	void CreateSpotLightResource();

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

	// 球体頂点生成
	void GenerateSphereVertices();
	VertexData CalculateSphereVertex(float lat, float lon, float u, float v) const;

	// 描画ヘルパー
	void BindVertexBuffer();
	void SetMaterialCBV();
	void SetLightingCBVs();
	void SetTextureSRVs();

	// Model共通データ
	Model* model_ = nullptr;

	// Objファイルのデータ
	ModelData modelData_;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource_;

	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData_ = nullptr;
	Material* materialData_ = nullptr;
	WorldTransformationMatrix* worldTransformationMatrixData_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;
	// カメラにデータを書き込む
	CameraForGPU* cameraData_ = nullptr;
	PointLight* pointLightData_ = nullptr;
	SpotLight* spotLightData_ = nullptr;

	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	// ワールド変換
	WorldTransform worldTransform;

	// カメラ
	Camera* camera_ = nullptr;

	// 球体の分割数
	uint32_t kSubdivision_ = Object3dConstants::kSphereSubdivision;

	// 緯度・経度の分割数に応じた角度の計算
	float kLatEvery_ = std::numbers::pi_v<float> / float(kSubdivision_);
	float kLonEvery_ = 2.0f * std::numbers::pi_v<float> / float(kSubdivision_);

	// 球体の頂点数の計算
	uint32_t totalVertexCount_ = kSubdivision_ * kSubdivision_ * Object3dConstants::kSphereVerticesPerQuad;

	// ファイルパス
	std::string filePath_;

	// スカイボックスのGPUハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE skyboxGpuHandle_{};
};


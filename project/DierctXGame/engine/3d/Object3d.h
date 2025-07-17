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

class Object3dCommon;


// 3Dオブジェクト
class Object3d
{
public:

	
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};


	// 座標変換行列データ
	struct worldTransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
		Matrix4x4 WorldInverseTranspose;
	};

	struct DirectionalLight {
		Vector4 color; // ライトの色
		Vector3 direction; // ライトの向き
		float intensity; // 輝度
	};

	
	struct ModelData {
		std::vector<VertexData> vertices;
		MaterialData material;
	};

	struct CameraForGPU {
		Vector3 worldPosition; // カメラのワールド座標
	};

	struct PointLight {
		Vector4 color; // ライトの色
		Vector3 position; // ライトの位置
		float intensity; // 輝度
		float radius; // ライトの届く最大距離
		float decay; // 減衰率
	};

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
	};;

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

	// カメラリソースの作成
	void CreateCameraResource();

	// ポイントライトの作成
	void CreatePointLightResource();

	// スポットライトの作成
	void CreateSpotLightResource();

	void DrawImGui();

	// ゲッター
	const Vector3& GetScale() const { return worldTransform.scale_; }
	const Vector3& GetRotate() const { return worldTransform.rotate_; }
	const Vector3& GetTranslate() const { return worldTransform.translate_; }

	// セッター
	void SetModel(Model* model) { this->model = model; }
	void SetModel(const std::string& filePath);
	void SetScale(const Vector3& scale) { worldTransform.scale_ = scale; }
	void SetRotate(const Vector3& rotate) { worldTransform.rotate_ = rotate; }
	void SetTranslate(const Vector3& translate) { worldTransform.translate_ = translate; }
	void SetWorldTransform(const WorldTransform& worldTransform) { this->worldTransform = worldTransform; }
	void SetCamera(Camera* camera) { this->camera = camera; }
	void SetTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) { textureHandle_ = textureHandle; }
private:
	// BufferResourceの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes);
	void CreateVertexData();
	void CreateMaterialData();
	void CreateDirectionalLightData();

	Model* model = nullptr;
	//// Objファイルのデータ
	ModelData modelData;

	//// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource;

	//// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	Material* materialData = nullptr;
	worldTransformationMatrix* worldTransformationMatrixData = nullptr;
	DirectionalLight* directionalLightData = nullptr;
	// カメラにデータを書き込む
	CameraForGPU* cameraData = nullptr;
	PointLight* pointLightData = nullptr;
	SpotLight* spotLightData = nullptr;

	// バッファリソースの使い道を補足するバッファビュー
	//// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	WorldTransform worldTransform;

	Camera* camera = nullptr;

	// 分割数
	uint32_t kSubdivision = 32;

	// 緯度・経度の分割数に応じた角度の計算
	float kLatEvery = std::numbers::pi_v<float> / float(kSubdivision);
	float kLonEvery = 2.0f * std::numbers::pi_v<float> / float(kSubdivision);

	// 球体の頂点数の計算
	uint32_t TotalVertexCount = kSubdivision * kSubdivision * 6;

	// 環境マップテクスチャ
	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle_;
};


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

	// カメラリソースの作成
	void CreateCameraResource();

	// ポイントライトの作成
	void CreatePointLightResource();

	// スポットライトの作成
	void CreateSpotLightResource();

	// ImGui描画
	void DrawImGui();

	/*------ゲッター------*/

	// スケールの取得
	const Vector3& GetScale() const { return worldTransform.scale_; }

	// 回転の取得
	const Vector3& GetRotate() const { return worldTransform.rotate_; }

	// 座標の取得
	const Vector3& GetTranslate() const { return worldTransform.translate_; }

	/*------セッター------*/

	// モデルの設定
	void SetModel(Model* model) { model_ = model; }

	// モデルの設定（ファイルパスから読み込み）
	void SetModel(const std::string& filePath);

	// スケールの設定
	void SetScale(const Vector3& scale) { worldTransform.scale_ = scale; }

	// 回転の設定
	void SetRotate(const Vector3& rotate) { worldTransform.rotate_ = rotate; }

	// 座標の設定
	void SetTranslate(const Vector3& translate) { worldTransform.translate_ = translate; }

	// ワールド変換の設定
	void SetWorldTransform(const WorldTransform& worldTransform) { this->worldTransform = worldTransform; }

	// カメラの設定
	void SetCamera(Camera* camera) { camera_ = camera; }

	// スカイボックスのファイルパス設定
	void SetSkyboxFilePath(std::string filePath);

	// ライトの明るさのセット
	void SetPointLight(float intensity) { pointLightData_->intensity = intensity; }

	void SetSpotLight(float intensity) { spotLightData_->intensity = intensity; }

	void SetDirectionalLight(float intensity) { directionalLightData_->intensity = intensity; }

	// マテリアルデータのセット
	void SetMaterialColor(Vector4 color) { materialData_->color = color; }

private:
	// BufferResourceの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes);
	void CreateVertexData();
	void CreateMaterialData();
	void CreateDirectionalLightData();

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

	// バッファリソースの使い道を補足するバッファビュー
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	WorldTransform worldTransform;

		Camera* camera_ = nullptr;

	// 分割数
	uint32_t kSubdivision_ = 32;

	// 緯度・経度の分割数に応じた角度の計算
	float kLatEvery_ = std::numbers::pi_v<float> / float(kSubdivision_);
	float kLonEvery_ = 2.0f * std::numbers::pi_v<float> / float(kSubdivision_);

	// 球体の頂点数の計算
	uint32_t totalVertexCount_ = kSubdivision_ * kSubdivision_ * 6;

	// ファイル名
	std::string filePath_;

	// 追加
	D3D12_GPU_DESCRIPTOR_HANDLE skyboxGpuHandle_{};
};


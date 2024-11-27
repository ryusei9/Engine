#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <vector>
#include "string"
#include "fstream"
#include "Matrix4x4.h"
#include "Transform.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

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

	struct Material {
		Vector4 color;
		int32_t enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};

	// 座標変換行列データ
	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};

	struct DirectionalLight {
		Vector4 color; // ライトの色
		Vector3 direction; // ライトの向き
		float intensity; // 輝度
	};

	struct MaterialData {
		std::string textureFilePath;
		uint32_t textureIndex = 0;
	};
	struct ModelData {
		std::vector<VertexData> vertices;
		MaterialData material;
	};

	// 初期化
	void Initialize(Object3dCommon* object3dCommon);

	// 更新
	void Update();

	// 描画
	void Draw();

	// .mtlファイルの読み込み
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	// .objファイルの読み取り
	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);
private:
	// BufferResourceの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes);
	void CreateVertexData();
	void CreateMaterialData();
	void CreateWVPData();
	void CreateDirectionalLightData();

	Object3dCommon* object3dCommon_ = nullptr;
	// Objファイルのデータ
	ModelData modelData;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	Material* materialData = nullptr;
	TransformationMatrix* transformationMatrixData = nullptr;
	DirectionalLight* directionalLightData = nullptr;

	// バッファリソースの使い道を補足するバッファビュー
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	Transform transform;
	Transform cameraTransform;
};


#pragma once
#include "ModelCommon.h"
#include "ModelData.h"
#include "Matrix4x4.h"
#include <vector>
#include "string"
#include "fstream"
#include "Material.h"
/// <summary>
/// 3Dモデル
/// </summary>
class Model
{
public:
	
	// 初期化
	void Initialize(ModelCommon* modelCommon,const std::string& directorypath,const std::string& filename);

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

	ModelCommon* modelCommon_;

	// Objファイルのデータ
	ModelData modelData;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;

	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	Material* materialData = nullptr;

	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
};


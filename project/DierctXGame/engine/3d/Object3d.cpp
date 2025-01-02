#include "Object3d.h"
#include "Object3dCommon.h"
#include "MakeIdentity4x4.h"
#include "Normalize.h"
#include "TextureManager.h"
#include "MakeAffineMatrix.h"
#include "MakeOrthographicMatrix.h"
#include "Multiply.h"
#include "Inverse.h"
#include "MakePerspectiveFovMatrix.h"
#include "ModelManager.h"

void Object3d::Initialize(Object3dCommon* object3dCommon)
{
	// 引数で受け取ってメンバ変数に記録する
	object3dCommon_ = object3dCommon;

	//// モデル読み込み
	//modelData = LoadObjFile("resources", "plane.obj");
	/*CreateVertexData();
	CreateMaterialData();*/
	CreateWVPData();
	CreateDirectionalLightData();
	// .objの参照しているテクスチャファイル読み込み
	//TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);
	// 読み込んだテクスチャの番号を取得
	//modelData.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData.material.textureFilePath);
	// Transform変数を作る
	transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	//cameraTransform = { {1.0f,1.0f,1.0f},{0.3f,0.0f,0.0f},{0.0f,4.0f,-10.0f} };
	this->camera = object3dCommon->GetDefaultCamera();
}

void Object3d::Update()
{
	Matrix4x4 worldMatrix = MakeAffineMatrix::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 worldViewProjectionMatrix;
	if (camera) {
		const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	}else {
		worldViewProjectionMatrix = worldMatrix;
	}

	transformationMatrixData->WVP = worldViewProjectionMatrix;
	transformationMatrixData->World = worldMatrix;
}

void Object3d::Draw()
{
	//// VBVを設定
	//object3dCommon_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	//// マテリアルCBufferの場所を設定

	//// 第一引数の0はRootParameter配列の0番目
	//object3dCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	// wvp用のCBufferの場所を設定
	object3dCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());

	/*object3dCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(modelData.material.textureIndex));*/

	// 平行光源
	object3dCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

	// 3Dモデルが割り当てられていれば描画する
	if (model) {
		model->Draw();
	}
	//// 描画 (DrawCall)。3頂点で1つのインスタンス。
	//object3dCommon_->GetDxCommon()->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
}

Object3d::MaterialData Object3d::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
	// 変数の宣言
	// 構築するMaterialData
	MaterialData materialData;
	// ファイルから読んだ一行を格納するもの
	std::string line;
	// ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);
	// 開けなかったら止める
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;

			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}
	return materialData;
}

Object3d::ModelData Object3d::LoadObjFile(const std::string& directoryPath, const std::string& filename)
{
	// 変数の宣言
	// 構築するModelData
	ModelData modelData;
	// 位置
	std::vector<Vector4> positions;
	// 法線
	std::vector<Vector3> normals;
	// テクスチャ座標
	std::vector<Vector2> texcoords;
	// ファイルから読んだ一行を格納するもの
	std::string line;

	// ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);
	// 開けなかったら止める
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		// 先頭の識別子を読む
		s >> identifier;

		// identifierに応じた処理
		// 頂点位置
		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.x *= -1.0f;
			position.w = 1.0f;
			positions.push_back(position);
			// 頂点テクスチャ座標
		} else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
			// 頂点法線
		} else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f;
			normals.push_back(normal);
			// 面
		} else if (identifier == "f") {
			VertexData triangle[3];
			// 面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					// /区切りでインデックスを読んでいく
					std::getline(v, index, '/');
					elementIndices[element] = std::stoi(index);
				}
				// 要素へのIndexから、実際の要素の値を取得して、頂点を構築していく
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				/*VertexData vertex = { position,texcoord,normal };
				modelData.vertices.push_back(vertex);*/
				triangle[faceVertex] = { position,texcoord,normal };
			}
			// 頂点を逆順で登録することで、回り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		} else if (identifier == "mtllib") {
			// materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;

			// 基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	return modelData;
}

void Object3d::SetModel(const std::string& filePath)
{
	// モデルを検索してセットする
	model = ModelManager::GetInstance()->FindModel(filePath);
}

Microsoft::WRL::ComPtr<ID3D12Resource> Object3d::CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes)
{
	// DXGIファクトリーの生成
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	// 関数が成功したかどうか
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};

	// UploadHeapを使う
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	// 頂点リソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};

	// バッファリソース。テクスチャの場合はまた別の設定をする
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	// リソースのサイズ
	resourceDesc.Width = sizeInBytes;

	// バッファの場合はこれらは1にする決まり
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;

	// バッファの場合はこれにする決まり
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;



	// 実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

//void Object3d::CreateVertexData()
//{
//	vertexResource = CreateBufferResource(object3dCommon_->GetDxCommon()->GetDevice(), sizeof(VertexData) * 6);
//
//	// リソースの先頭のアドレスから使う
//	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
//	// 使用するリソースのサイズは頂点6つ分のサイズ
//	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6;
//	// 1頂点当たりのサイズ
//	vertexBufferView.StrideInBytes = sizeof(VertexData);
//
//
//	// 書き込むためのアドレスを取得
//	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
//
//	// 頂点データをリソースにコピー
//	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
//
//}
//
//void Object3d::CreateMaterialData()
//{
//	materialResource = CreateBufferResource(object3dCommon_->GetDxCommon()->GetDevice(), sizeof(Material));
//
//	// ...Mapしてデータを書き込む。色は白を設定しておくといい
//	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
//
//	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
//
//	// SpriteはLightingしないのでfalseを設定する
//	materialData->enableLighting = false;
//
//	// 単位行列で初期化
//	materialData->uvTransform = MakeIdentity4x4::MakeIdentity4x4();
//}

void Object3d::CreateWVPData()
{
	wvpResource = CreateBufferResource(object3dCommon_->GetDxCommon()->GetDevice(), sizeof(TransformationMatrix));

	// 書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));

	// 単位行列を書き込んでおく
	transformationMatrixData->WVP = MakeIdentity4x4::MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4::MakeIdentity4x4();
}

void Object3d::CreateDirectionalLightData()
{
	directionalLightResource = CreateBufferResource(object3dCommon_->GetDxCommon()->GetDevice(), sizeof(DirectionalLight));

	// 書き込むためのアドレスを取得
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));

	// 真上から白いライトで照らす
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightData->intensity = 1.0f;

	// 正規化
	Normalize(directionalLightData->direction);
}

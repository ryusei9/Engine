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
#include <imgui.h>

void Object3d::Initialize(const std::string& fileName)
{
	// 引数で受け取ってメンバ変数に記録する
	//object3dCommon_ = object3dCommon;

	//// モデル読み込み
	modelData = LoadObjFile("resources", fileName);
	CreateVertexData();
	CreateMaterialData();
	CreateDirectionalLightData();
	// .objの参照しているテクスチャファイル読み込み
	TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);
	// 読み込んだテクスチャの番号を取得
	modelData.material.gpuHandle = TextureManager::GetInstance()->GetSrvHandleGPU(modelData.material.textureFilePath);
	// Transform変数を作る
	worldTransform.Initialize();
	//cameraTransform = { {1.0f,1.0f,1.0f},{0.3f,0.0f,0.0f},{0.0f,4.0f,-10.0f} };
	this->camera = Object3dCommon::GetInstance()->GetDefaultCamera();

	// カメラリソースの作成
	CreateCameraResource();
	// ポイントライトの作成
	CreatePointLightResource();
	// スポットライトの作成
	CreateSpotLightResource();
}

void Object3d::Update()
{
	worldTransform.Update();
}

void Object3d::Draw()
{

	//// VBVを設定
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	//// マテリアルCBufferの場所を設定

	//// 第一引数の0はRootParameter配列の0番目
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	worldTransform.SetPipeline();


	// ディスクリプタヒープに関連付けられたハンドルを使用
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, modelData.material.gpuHandle);

	// 平行光源
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());

	// ポイントライト
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(5, pointLightResource->GetGPUVirtualAddress());

	// スポットライト
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(6, spotLightResource->GetGPUVirtualAddress());
	// 3Dモデルが割り当てられていれば描画する
	if (model) {
		model->Draw();
	}
	//// 描画 (DrawCall)。3頂点で1つのインスタンス。
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
}

MaterialData Object3d::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
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

void Object3d::CreateCameraResource()
{
	// カメラのリソースを作成
	cameraResource = CreateBufferResource(Object3dCommon::GetInstance()->GetDxCommon()->GetDevice(), sizeof(CameraForGPU));
	cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraData));
	// カメラの初期位置
	cameraData->worldPosition = camera->GetTranslate();
}

void Object3d::CreatePointLightResource()
{
	// ポイントライトのリソースを作成
	pointLightResource = CreateBufferResource(Object3dCommon::GetInstance()->GetDxCommon()->GetDevice(), sizeof(PointLight));
	pointLightResource->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData));
	// ポイントライトの初期位置
	pointLightData->position = { 0.0f, 2.0f, 0.0f };
	pointLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	pointLightData->intensity = 1.0f;
	pointLightData->radius = 10.0f;					 // ポイントライトの有効範囲
	pointLightData->decay = 1.0f;						 // ポイントライトの減衰率
	pointLightResource->Unmap(0, nullptr);
}

void Object3d::CreateSpotLightResource()
{
	// スポットライトのリソースを作成
	spotLightResource = CreateBufferResource(Object3dCommon::GetInstance()->GetDxCommon()->GetDevice(), sizeof(SpotLight));
	spotLightResource->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData));
	// スポットライトの初期位置
	spotLightData->position = { 0.0f, 2.0f, 0.0f };
	spotLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	spotLightData->intensity = 4.0f;
	spotLightData->direction = { 0.0f, -1.0f, 0.0f };
	spotLightData->cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);	 // スポットライトの角度
	spotLightData->cosFalloffStart = std::cos(std::numbers::pi_v<float> / 6.0f); // スポットライトの開始角度の余弦値
	spotLightData->distance = 7.0f;					 // スポットライトの有効範囲
	spotLightData->decay = 2.0f;						 // スポットライトの減衰率
	spotLightResource->Unmap(0, nullptr);
}

void Object3d::DrawImGui()
{
	// ImGuiのウィンドウを作成
	ImGui::Begin("Object3d");
	ImGui::ColorEdit4("color", &materialData->color.x);
	ImGui::ColorEdit4("lightColor", &directionalLightData->color.x);
	ImGui::DragFloat3("lightDirection", &directionalLightData->direction.x, 0.01f);
	ImGui::DragFloat("intensity", &directionalLightData->intensity, 0.01f);

	/*------ポイントライト------*/
	ImGui::ColorEdit4("pointLightColor", &pointLightData->color.x);
	ImGui::DragFloat3("pointLightPosition", &pointLightData->position.x, 0.01f);
	ImGui::DragFloat("pointLightIntensity", &pointLightData->intensity, 0.01f);

	/*------スポットライト------*/
	ImGui::ColorEdit4("spotLightColor", &spotLightData->color.x);
	ImGui::DragFloat3("spotLightPosition", &spotLightData->position.x, 0.01f);
	ImGui::DragFloat3("spotLightDirection", &spotLightData->direction.x, 0.01f);
	ImGui::DragFloat("spotLightIntensity", &spotLightData->intensity, 0.01f);
	ImGui::DragFloat("spotLightDistance", &spotLightData->distance, 0.01f);
	ImGui::DragFloat("spotLightDecay", &spotLightData->decay, 0.01f);
	ImGui::DragFloat("spotLightCosAngle", &spotLightData->cosAngle, 0.01f);
	ImGui::DragFloat("spotLightCosFalloffStart", &spotLightData->cosFalloffStart, 0.01f);


	// ウィンドウを閉じる
	ImGui::End();
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

void Object3d::CreateVertexData()
{
	vertexResource = CreateBufferResource(Object3dCommon::GetInstance()->GetDxCommon()->GetDevice(), sizeof(VertexData) * (modelData.vertices.size() + TotalVertexCount));

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * (modelData.vertices.size() + TotalVertexCount));
	// 1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);


	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// 頂点データをリソースにコピー
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	// アンマップ
	vertexResource->Unmap(0, nullptr);
	// 球体の頂点データをコピー
	VertexData* sphereVertexData = vertexData + modelData.vertices.size();
	auto calculateVertex = [](float lat, float lon, float u, float v) {
		VertexData vertex;
		vertex.position = { cos(lat) * cos(lon), sin(lat), cos(lat) * sin(lon), 1.0f };
		vertex.texcoord = { u, v };
		vertex.normal = { vertex.position.x, vertex.position.y, vertex.position.z };
		return vertex;
		};
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex; // θ
		float nextLat = lat + kLatEvery;
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float u = float(lonIndex) / float(kSubdivision);
			float v = 1.0f - float(latIndex) / float(kSubdivision);
			float lon = lonIndex * kLonEvery; // Φ
			float nextLon = lon + kLonEvery;
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
			// 6つの頂点を計算
			sphereVertexData[start + 0] = calculateVertex(lat, lon, u, v);
			sphereVertexData[start + 1] = calculateVertex(nextLat, lon, u, v - 1.0f / float(kSubdivision));
			sphereVertexData[start + 2] = calculateVertex(lat, nextLon, u + 1.0f / float(kSubdivision), v);
			sphereVertexData[start + 3] = calculateVertex(nextLat, nextLon, u + 1.0f / float(kSubdivision), v - 1.0f / float(kSubdivision));
			sphereVertexData[start + 4] = calculateVertex(lat, nextLon, u + 1.0f / float(kSubdivision), v);
			sphereVertexData[start + 5] = calculateVertex(nextLat, lon, u, v - 1.0f / float(kSubdivision));
		}
	}
}

void Object3d::CreateMaterialData()
{
	materialResource = CreateBufferResource(Object3dCommon::GetInstance()->GetDxCommon()->GetDevice(), sizeof(Material));

	// ...Mapしてデータを書き込む。色は白を設定しておくといい
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	// Lightingはtrueを設定する
	materialData->enableLighting = true;

	materialData->shininess = 50.0f;

	// 単位行列で初期化
	materialData->uvTransform = MakeIdentity4x4::MakeIdentity4x4();
}

void Object3d::CreateDirectionalLightData()
{
	directionalLightResource = CreateBufferResource(Object3dCommon::GetInstance()->GetDxCommon()->GetDevice(), sizeof(DirectionalLight));

	// 書き込むためのアドレスを取得
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));

	// 真上から白いライトで照らす
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightData->intensity = 1.0f;


	// 正規化
	Normalize::Normalize(directionalLightData->direction);
}

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
#include "DirectXCommon.h"

//
// Object3d
// - 単一の 3D オブジェクトを表すクラス実装。
// - OBJ ファイルの読み込み・頂点/マテリアルバッファ作成、描画、GUI 操作を含む。
// - カメラ／ライト用の定数バッファも保持し、必要時に GPU に渡す。
// - 注意: ファイル入出力やメモリマップは失敗時に assert で停止する実装になっているため、
//         実行環境ではファイルの存在や DirectX の初期化が前提。
//

void Object3d::Initialize(const std::string& fileName)
{
	// ファイル名からモデルを読み込み、GPU に転送する初期化処理を行う。
	// 呼び出し側: シーンの初期化時など。resources ディレクトリ下のファイル名を期待する。
	// 副作用:
	//  - vertexResource / materialResource 等の Upload バッファを作成して Map する
	//  - TextureManager によるテクスチャ読み込みを行う
	//  - Skybox 用テクスチャのハンドルをキャッシュする
	//  - worldTransform を初期化し、カメラ参照を取得する
	//  - カメラ/ポイントライト/スポットライト用バッファを作成する
	//
	modelData = LoadObjFile("resources", fileName);
	CreateVertexData();
	CreateMaterialData();
	CreateDirectionalLightData();

	// OBJ が参照するテクスチャを事前登録
	TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);

	// デフォルトの skybox テクスチャを読み込み、GPU ハンドルを取得して保持
	filePath_ = "resources/skybox.dds";
	TextureManager::GetInstance()->LoadTexture(filePath_);
	skyboxGpuHandle_ = TextureManager::GetInstance()->GetSrvHandleGPU(filePath_);

	// モデルマテリアルの SRV ハンドルを保持（描画時に使う）
	modelData.material.gpuHandle = TextureManager::GetInstance()->GetSrvHandleGPU(modelData.material.textureFilePath);

	// ワールド変換の初期化（位置/回転/スケール）
	worldTransform.Initialize();

	// デフォルトカメラの参照を取得
	this->camera = Object3dCommon::GetInstance()->GetDefaultCamera();

	// カメラ・ライト用の GPU 定数バッファを作成
	CreateCameraResource();
	CreatePointLightResource();
	CreateSpotLightResource();
}

void Object3d::Update()
{
	// ワールド変換の内部状態更新（行列再計算など）
	// - 外部から worldTransform の translate/rotate/scale を変更したあとに呼ぶ
	worldTransform.Update();

	// カメラ / ライトのデータは必要に応じて ImGui 等で変更されるため、
	// GPU バッファへ書き戻す処理は描画前または ImGui ボタン処理側で行う想定。
}

void Object3d::Draw()
{
	// 描画準備: 頂点バッファをバインドし、マテリアル CBV・テクスチャ SRV をルートに設定する
	// 前提: Draw 呼び出し前に適切な PSO / RootSignature / DescriptorHeap が設定済みであること。

	// 頂点バッファビューをバインド
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);

	// マテリアル用 CBV をルートに設定 (b0)
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	// worldTransform の行列等をパイプラインにセット（ワールド行列等をマテリアルに反映）
	worldTransform.SetPipeline();

	// モデルのテクスチャ SRV をデスクリプタテーブルへ設定 (slot = 2 を想定)
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, modelData.material.gpuHandle);

	// ディレクショナルライト（CBV slot = 3）
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

	// カメラ情報（CBV slot = 4）
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());

	// ポイントライト（CBV slot = 5）
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(5, pointLightResource->GetGPUVirtualAddress());

	// スポットライト（CBV slot = 6）
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(6, spotLightResource->GetGPUVirtualAddress());

	// スカイボックスが設定されていれば SRV をセット (slot = 7)
	if (skyboxGpuHandle_.ptr != 0) {
		Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(7, skyboxGpuHandle_);
	}

	// モデルが外部管理 (ModelManager 経由) でセットされていればそれを使って描画
	if (model) {
		model->Draw();
	}

	// 追加の DrawInstanced（fallback / 互換性のため）
	// - model->Draw() が呼ばれた場合は二重描画にならないよう、用途に応じてどちらかを使うこと
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
}

MaterialData Object3d::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
	// MTL を読み込み、map_Kd を検出して MaterialData.textureFilePath を設定して返す
	MaterialData materialData;
	std::string line;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}
	return materialData;
}

ModelData Object3d::LoadObjFile(const std::string& directoryPath, const std::string& filename)
{
	// OBJ ファイルをパースして ModelData を構築する
	// サポートするトークン: v, vt, vn, f, mtllib
	ModelData modelData;
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::string line;

	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "v") {
			// 頂点位置 (x,y,z) を読み込み、右手系->左手系補正のため X を反転して格納
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.x *= -1.0f;
			position.w = 1.0f;
			positions.push_back(position);
		} else if (identifier == "vt") {
			// テクスチャ座標 (u,v) を読み込み、V を反転して DirectX 形式に合わせる
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		} else if (identifier == "vn") {
			// 法線 (x,y,z) を読み込み、X を反転して格納
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f;
			normals.push_back(normal);
		} else if (identifier == "f") {
			// 面 (三角形) の読み取り: 各頂点は "posIndex/uvIndex/normalIndex"
			VertexData triangle[3];
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');
					elementIndices[element] = std::stoi(index); // OBJ は 1 始まり
				}
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				triangle[faceVertex] = { position, texcoord, normal };
			}
			// 頂点の登録順を逆順にする（回り順を調整）
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		} else if (identifier == "mtllib") {
			// マテリアルライブラリ参照を読み込む
			std::string materialFilename;
			s >> materialFilename;
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}

	return modelData;
}

void Object3d::CreateCameraResource()
{
	// カメラ情報用の定数バッファを作成して Map
	cameraResource = CreateBufferResource(Object3dCommon::GetInstance()->GetDxCommon()->GetDevice(), sizeof(CameraForGPU));
	cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraData));
	// 初期値として現在のカメラ位置をコピー
	cameraData->worldPosition = camera->GetTranslate();
}

void Object3d::CreatePointLightResource()
{
	// ポイントライト用定数バッファを作成して Map、デフォルト値をセットする
	pointLightResource = CreateBufferResource(Object3dCommon::GetInstance()->GetDxCommon()->GetDevice(), sizeof(PointLight));
	pointLightResource->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData));

	pointLightData->position = { 0.0f, 2.0f, 0.0f };
	pointLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	pointLightData->intensity = 1.0f;
	pointLightData->radius = 10.0f;
	pointLightData->decay = 1.0f;

	// 書き込み終了後に Unmap（Upload ヒープの扱いはいくつかの実装で省略されるが明示的に行う）
	pointLightResource->Unmap(0, nullptr);
}

void Object3d::CreateSpotLightResource()
{
	// スポットライト用定数バッファを作成して Map、デフォルト値をセットする
	spotLightResource = CreateBufferResource(Object3dCommon::GetInstance()->GetDxCommon()->GetDevice(), sizeof(SpotLight));
	spotLightResource->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData));

	spotLightData->position = { 0.0f, 2.0f, 0.0f };
	spotLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	spotLightData->intensity = 4.0f;
	spotLightData->direction = { 0.0f, -1.0f, 0.0f };
	spotLightData->cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
	spotLightData->cosFalloffStart = std::cos(std::numbers::pi_v<float> / 6.0f);
	spotLightData->distance = 7.0f;
	spotLightData->decay = 2.0f;

	spotLightResource->Unmap(0, nullptr);
}

void Object3d::DrawImGui()
{
#ifdef USE_IMGUI
	// ImGui によるデバッグ用 GUI（位置・回転・スケール、マテリアル/ライトパラメータの編集）
	ImGui::Begin("Object3d");
	ImGui::DragFloat3("position", &worldTransform.translate_.x, 0.01f);
	ImGui::DragFloat3("rotation", &worldTransform.rotate_.x, 0.01f);
	ImGui::DragFloat3("scale", &worldTransform.scale_.x, 0.01f);

	ImGui::ColorEdit4("color", &materialData->color.x);
	ImGui::ColorEdit4("lightColor", &directionalLightData->color.x);
	ImGui::DragFloat3("lightDirection", &directionalLightData->direction.x, 0.01f);
	ImGui::DragFloat("intensity", &directionalLightData->intensity, 0.01f);
	ImGui::SliderFloat("environmentCoefficient", &materialData->environmentCoefficient, 0.0f, 1.0f);

	// ポイントライト
	ImGui::ColorEdit4("pointLightColor", &pointLightData->color.x);
	ImGui::DragFloat3("pointLightPosition", &pointLightData->position.x, 0.01f);
	ImGui::DragFloat("pointLightIntensity", &pointLightData->intensity, 0.01f);

	// スポットライト
	ImGui::ColorEdit4("spotLightColor", &spotLightData->color.x);
	ImGui::DragFloat3("spotLightPosition", &spotLightData->position.x, 0.01f);
	ImGui::DragFloat3("spotLightDirection", &spotLightData->direction.x, 0.01f);
	ImGui::DragFloat("spotLightIntensity", &spotLightData->intensity, 0.01f);
	ImGui::DragFloat("spotLightDistance", &spotLightData->distance, 0.01f);
	ImGui::DragFloat("spotLightDecay", &spotLightData->decay, 0.01f);
	ImGui::DragFloat("spotLightCosAngle", &spotLightData->cosAngle, 0.01f);
	ImGui::DragFloat("spotLightCosFalloffStart", &spotLightData->cosFalloffStart, 0.01f);

	ImGui::End();
#endif
}

void Object3d::SetModel(const std::string& filePath)
{
	// ModelManager から既にロード済みのモデルを参照 (nullptr チェックは呼び出し側で)
	model = ModelManager::GetInstance()->FindModel(filePath);
}

void Object3d::SetSkyboxFilePath(std::string filePath)
{
	// スカイボックス用テクスチャを設定し、SRV ハンドルを更新する
	filePath_ = filePath;
	TextureManager::GetInstance()->LoadTexture(filePath_);
	skyboxGpuHandle_ = TextureManager::GetInstance()->GetSrvHandleGPU(filePath_);
}

Microsoft::WRL::ComPtr<ID3D12Resource> Object3d::CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes)
{
	// Upload ヒープ上のバッファを作成して返すユーティリティ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	// ファクトリは現状特に使用していないが、取得しておく
	(void)dxgiFactory;

	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

void Object3d::CreateVertexData()
{
	// 頂点バッファサイズはモデル頂点 + 球体用追加領域 (TotalVertexCount)
	vertexResource = CreateBufferResource(Object3dCommon::GetInstance()->GetDxCommon()->GetDevice(), sizeof(VertexData) * (modelData.vertices.size() + TotalVertexCount));

	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * (modelData.vertices.size() + TotalVertexCount));
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// マップしてモデル頂点データをコピー
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	// アンマップしてから球体の頂点領域に直接書き込む実装にしている（既存の実装に合わせる）
	vertexResource->Unmap(0, nullptr);

	// 球体用頂点データの計算と書き込み
	VertexData* sphereVertexData = vertexData + modelData.vertices.size();
	auto calculateVertex = [](float lat, float lon, float u, float v) {
		VertexData vertex;
		vertex.position = { cos(lat) * cos(lon), sin(lat), cos(lat) * sin(lon), 1.0f };
		vertex.texcoord = { u, v };
		vertex.normal = { vertex.position.x, vertex.position.y, vertex.position.z };
		return vertex;
	};
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;
		float nextLat = lat + kLatEvery;
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float u = float(lonIndex) / float(kSubdivision);
			float v = 1.0f - float(latIndex) / float(kSubdivision);
			float lon = lonIndex * kLonEvery;
			float nextLon = lon + kLonEvery;
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
			// 6 頂点分を計算して配置
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
	// material 用定数バッファを作成して Map、初期値を設定する
	materialResource = CreateBufferResource(Object3dCommon::GetInstance()->GetDxCommon()->GetDevice(), sizeof(Material));
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = true;
	materialData->shininess = 50.0f;
	materialData->uvTransform = MakeIdentity4x4::MakeIdentity4x4();
	materialData->environmentCoefficient = 0.0f;
}

void Object3d::CreateDirectionalLightData()
{
	// ディレクショナルライト用定数バッファを作成して Map、初期値をセットする
	directionalLightResource = CreateBufferResource(Object3dCommon::GetInstance()->GetDxCommon()->GetDevice(), sizeof(DirectionalLight));
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));

	// 初期設定: 上方向からの強めの白色ライト
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightData->intensity = 3.0f;

	// 正規化して方向ベクトルを単位化
	Normalize::Normalize(directionalLightData->direction);
}

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
// 

void Object3d::Initialize(const std::string& fileName)
{
	// OBJファイルを読み込み
	modelData_ = LoadObjFile(Object3dConstants::kDefaultResourceDirectory, fileName);
	
	// リソース作成
	CreateVertexData();
	CreateMaterialData();
	CreateDirectionalLightData();

	// テクスチャの読み込み
	TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);

	// デフォルトのスカイボックステクスチャを読み込み
	filePath_ = Object3dConstants::kDefaultSkyboxFilePath;
	TextureManager::GetInstance()->LoadTexture(filePath_);
	skyboxGpuHandle_ = TextureManager::GetInstance()->GetSrvHandleGPU(filePath_);

	// マテリアルのGPUハンドル取得
	modelData_.material.gpuHandle = TextureManager::GetInstance()->GetSrvHandleGPU(modelData_.material.textureFilePath);

	// ワールド変換の初期化
	worldTransform.Initialize();

	// デフォルトカメラの取得
	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();

	// ライト用リソース作成
	CreateCameraResource();
	CreatePointLightResource();
	CreateSpotLightResource();
}

void Object3d::Update()
{
	worldTransform.Update();
}

void Object3d::Draw()
{
	BindVertexBuffer();
	SetMaterialCBV();
	worldTransform.SetPipeline();
	SetTextureSRVs();
	SetLightingCBVs();

	// モデルが設定されていれば描画
	if (model_) {
		model_->Draw();
	}

	// フォールバック描画
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->DrawInstanced(
		UINT(modelData_.vertices.size()), 1, 0, 0);
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
		
		if (identifier == Object3dConstants::kMtlIdentifierTexture) {
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

		if (identifier == Object3dConstants::kObjIdentifierVertex) {
			Vector4 position = ParseVertexPosition(s);
			positions.push_back(position);
		}
		else if (identifier == Object3dConstants::kObjIdentifierTexCoord) {
			Vector2 texcoord = ParseTexCoord(s);
			texcoords.push_back(texcoord);
		}
		else if (identifier == Object3dConstants::kObjIdentifierNormal) {
			Vector3 normal = ParseNormal(s);
			normals.push_back(normal);
		}
		else if (identifier == Object3dConstants::kObjIdentifierFace) {
			ParseFace(s, positions, texcoords, normals, modelData);
		}
		else if (identifier == Object3dConstants::kObjIdentifierMaterialLib) {
			std::string materialFilename;
			s >> materialFilename;
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}

	return modelData;
}

void Object3d::CreateCameraResource()
{
	cameraResource_ = CreateBufferResource(
		Object3dCommon::GetInstance()->GetDxCommon()->GetDevice(), 
		sizeof(CameraForGPU));
	cameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));
	cameraData_->worldPosition = camera_->GetTranslate();
}

void Object3d::CreatePointLightResource()
{
	pointLightResource_ = CreateBufferResource(
		Object3dCommon::GetInstance()->GetDxCommon()->GetDevice(), 
		sizeof(PointLight));
	pointLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData_));

	// デフォルト値の設定
	pointLightData_->position = { 0.0f, Object3dConstants::kDefaultLightPositionY, 0.0f };
	pointLightData_->color = { 
		Object3dConstants::kDefaultMaterialColorR, 
		Object3dConstants::kDefaultMaterialColorG, 
		Object3dConstants::kDefaultMaterialColorB, 
		Object3dConstants::kDefaultMaterialColorA 
	};
	pointLightData_->intensity = Object3dConstants::kDefaultPointLightIntensity;
	pointLightData_->radius = Object3dConstants::kDefaultPointLightRadius;
	pointLightData_->decay = Object3dConstants::kDefaultPointLightDecay;

	pointLightResource_->Unmap(0, nullptr);
}

void Object3d::CreateSpotLightResource()
{
	spotLightResource_ = CreateBufferResource(
		Object3dCommon::GetInstance()->GetDxCommon()->GetDevice(), 
		sizeof(SpotLight));
	spotLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData_));

	// デフォルト値の設定
	spotLightData_->position = { 0.0f, Object3dConstants::kDefaultLightPositionY, 0.0f };
	spotLightData_->color = { 
		Object3dConstants::kDefaultMaterialColorR, 
		Object3dConstants::kDefaultMaterialColorG, 
		Object3dConstants::kDefaultMaterialColorB, 
		Object3dConstants::kDefaultMaterialColorA 
	};
	spotLightData_->intensity = Object3dConstants::kDefaultSpotLightIntensity;
	spotLightData_->direction = { 0.0f, Object3dConstants::kDefaultDirectionalLightDirectionY, 0.0f };
	spotLightData_->cosAngle = std::cos(std::numbers::pi_v<float> / Object3dConstants::kSpotLightAngleDivisor);
	spotLightData_->cosFalloffStart = std::cos(std::numbers::pi_v<float> / Object3dConstants::kSpotLightFalloffAngleDivisor);
	spotLightData_->distance = Object3dConstants::kDefaultSpotLightDistance;
	spotLightData_->decay = Object3dConstants::kDefaultSpotLightDecay;

	spotLightResource_->Unmap(0, nullptr);
}

void Object3d::DrawImGui()
{
#ifdef USE_IMGUI
	// ImGui によるデバッグ用 GUI（位置・回転・スケール、マテリアル/ライトパラメータの編集）
	ImGui::Begin("Object3d");
	ImGui::DragFloat3("position", &worldTransform.translate_.x, 0.01f);
	ImGui::DragFloat3("rotation", &worldTransform.rotate_.x, 0.01f);
	ImGui::DragFloat3("scale", &worldTransform.scale_.x, 0.01f);

	ImGui::ColorEdit4("color", &materialData_->color.x);
	ImGui::ColorEdit4("lightColor", &directionalLightData_->color.x);
	ImGui::DragFloat3("lightDirection", &directionalLightData_->direction.x, 0.01f);
	ImGui::DragFloat("intensity", &directionalLightData_->intensity, 0.01f);
	ImGui::SliderFloat("environmentCoefficient", &materialData_->environmentCoefficient, 0.0f, 1.0f);

	// ポイントライト
	ImGui::ColorEdit4("pointLightColor", &pointLightData_->color.x);
	ImGui::DragFloat3("pointLightPosition", &pointLightData_->position.x, 0.01f);
	ImGui::DragFloat("pointLightIntensity", &pointLightData_->intensity, 0.01f);

	// スポットライト
	ImGui::ColorEdit4("spotLightColor", &spotLightData_->color.x);
	ImGui::DragFloat3("spotLightPosition", &spotLightData_->position.x, 0.01f);
	ImGui::DragFloat3("spotLightDirection", &spotLightData_->direction.x, 0.01f);
	ImGui::DragFloat("spotLightIntensity", &spotLightData_->intensity, 0.01f);
	ImGui::DragFloat("spotLightDistance", &spotLightData_->distance, 0.01f);
	ImGui::DragFloat("spotLightDecay", &spotLightData_->decay, 0.01f);
	ImGui::DragFloat("spotLightCosAngle", &spotLightData_->cosAngle, 0.01f);
	ImGui::DragFloat("spotLightCosFalloffStart", &spotLightData_->cosFalloffStart, 0.01f);

	ImGui::End();
#endif
}

void Object3d::SetModel(const std::string& filePath)
{
	// ModelManager から既にロード済みのモデルを参照 (nullptr チェックは呼び出し側で)
	model_ = ModelManager::GetInstance()->FindModel(filePath);
}

void Object3d::SetSkyboxFilePath(std::string filePath)
{
	// スカイボックス用テクスチャを設定し、SRV ハンドルを更新する
	filePath_ = filePath;
	TextureManager::GetInstance()->LoadTexture(filePath_);
	skyboxGpuHandle_ = TextureManager::GetInstance()->GetSrvHandleGPU(filePath_);
}

Microsoft::WRL::ComPtr<ID3D12Resource> Object3d::CreateBufferResource(
	const Microsoft::WRL::ComPtr<ID3D12Device>& device, 
	size_t sizeInBytes)
{
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
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
	size_t totalSize = sizeof(VertexData) * (modelData_.vertices.size() + totalVertexCount_);
	vertexResource_ = CreateBufferResource(
		Object3dCommon::GetInstance()->GetDxCommon()->GetDevice(), 
		totalSize);

	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(totalSize);
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// モデル頂点データをコピー
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
	vertexResource_->Unmap(0, nullptr);

	// 球体頂点を生成
	GenerateSphereVertices();
}

void Object3d::CreateMaterialData()
{
	materialResource_ = CreateBufferResource(
		Object3dCommon::GetInstance()->GetDxCommon()->GetDevice(), 
		sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	// デフォルト値の設定
	materialData_->color = Vector4(
		Object3dConstants::kDefaultMaterialColorR, 
		Object3dConstants::kDefaultMaterialColorG, 
		Object3dConstants::kDefaultMaterialColorB, 
		Object3dConstants::kDefaultMaterialColorA);
	materialData_->enableLighting = Object3dConstants::kDefaultLightingEnabled;
	materialData_->shininess = Object3dConstants::kDefaultShininess;
	materialData_->uvTransform = MakeIdentity4x4::MakeIdentity4x4();
	materialData_->environmentCoefficient = Object3dConstants::kDefaultEnvironmentCoefficient;
}

void Object3d::CreateDirectionalLightData()
{
	directionalLightResource_ = CreateBufferResource(
		Object3dCommon::GetInstance()->GetDxCommon()->GetDevice(), 
		sizeof(DirectionalLight));
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));

	// デフォルト値の設定
	directionalLightData_->color = { 
		Object3dConstants::kDefaultMaterialColorR, 
		Object3dConstants::kDefaultMaterialColorG, 
		Object3dConstants::kDefaultMaterialColorB, 
		Object3dConstants::kDefaultMaterialColorA 
	};
	directionalLightData_->direction = { 0.0f, Object3dConstants::kDefaultDirectionalLightDirectionY, 0.0f };
	directionalLightData_->intensity = Object3dConstants::kDefaultLightIntensity;

	Normalize::Normalize(directionalLightData_->direction);
}

// ===== OBJパース用ヘルパー関数 =====

Vector4 Object3d::ParseVertexPosition(std::istringstream& stream)
{
	Vector4 position;
	stream >> position.x >> position.y >> position.z;
	position.x *= Object3dConstants::kCoordinateFlipScale;
	position.w = Object3dConstants::kDefaultPositionW;
	return position;
}

Vector2 Object3d::ParseTexCoord(std::istringstream& stream)
{
	Vector2 texcoord;
	stream >> texcoord.x >> texcoord.y;
	texcoord.y = 1.0f - texcoord.y;
	return texcoord;
}

Vector3 Object3d::ParseNormal(std::istringstream& stream)
{
	Vector3 normal;
	stream >> normal.x >> normal.y >> normal.z;
	normal.x *= Object3dConstants::kCoordinateFlipScale;
	return normal;
}

void Object3d::ParseFace(
	std::istringstream& stream,
	const std::vector<Vector4>& positions,
	const std::vector<Vector2>& texcoords,
	const std::vector<Vector3>& normals,
	ModelData& modelData)
{
	VertexData triangle[Object3dConstants::kFaceVertexCount];
	
	for (int32_t faceVertex = 0; faceVertex < Object3dConstants::kFaceVertexCount; ++faceVertex) {
		std::string vertexDefinition;
		stream >> vertexDefinition;

		uint32_t elementIndices[Object3dConstants::kFaceElementCount];
		ParseVertexIndices(vertexDefinition, elementIndices);

		Vector4 position = positions[elementIndices[0] - Object3dConstants::kObjIndexOffset];
		Vector2 texcoord = texcoords[elementIndices[1] - Object3dConstants::kObjIndexOffset];
		Vector3 normal = normals[elementIndices[2] - Object3dConstants::kObjIndexOffset];
		triangle[faceVertex] = { position, texcoord, normal };
	}
	
	// 頂点の登録順を逆にして回り順を調整
	modelData.vertices.push_back(triangle[2]);
	modelData.vertices.push_back(triangle[1]);
	modelData.vertices.push_back(triangle[0]);
}

void Object3d::ParseVertexIndices(const std::string& vertexDefinition, uint32_t* outIndices)
{
	std::istringstream v(vertexDefinition);
	
	for (int32_t element = 0; element < Object3dConstants::kFaceElementCount; ++element) {
		std::string index;
		std::getline(v, index, Object3dConstants::kFaceDelimiter);
		outIndices[element] = std::stoi(index);
	}
}

// ===== 球体頂点生成 =====

void Object3d::GenerateSphereVertices()
{
	VertexData* sphereVertexData = vertexData_ + modelData_.vertices.size();
	
	for (uint32_t latIndex = 0; latIndex < kSubdivision_; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery_ * latIndex;
		float nextLat = lat + kLatEvery_;
		
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision_; ++lonIndex) {
			float u = float(lonIndex) / float(kSubdivision_);
			float v = 1.0f - float(latIndex) / float(kSubdivision_);
			float lon = lonIndex * kLonEvery_;
			float nextLon = lon + kLonEvery_;
			
			uint32_t start = (latIndex * kSubdivision_ + lonIndex) * Object3dConstants::kSphereVerticesPerQuad;
			
			// 6頂点分を計算して配置
			sphereVertexData[start + 0] = CalculateSphereVertex(lat, lon, u, v);
			sphereVertexData[start + 1] = CalculateSphereVertex(nextLat, lon, u, v - 1.0f / float(kSubdivision_));
			sphereVertexData[start + 2] = CalculateSphereVertex(lat, nextLon, u + 1.0f / float(kSubdivision_), v);
			sphereVertexData[start + 3] = CalculateSphereVertex(nextLat, nextLon, u + 1.0f / float(kSubdivision_), v - 1.0f / float(kSubdivision_));
			sphereVertexData[start + 4] = CalculateSphereVertex(lat, nextLon, u + 1.0f / float(kSubdivision_), v);
			sphereVertexData[start + 5] = CalculateSphereVertex(nextLat, lon, u, v - 1.0f / float(kSubdivision_));
		}
	}
}

VertexData Object3d::CalculateSphereVertex(float lat, float lon, float u, float v) const
{
	VertexData vertex;
	vertex.position = { 
		std::cos(lat) * std::cos(lon), 
		std::sin(lat), 
		std::cos(lat) * std::sin(lon), 
		1.0f 
	};
	vertex.texcoord = { u, v };
	vertex.normal = { vertex.position.x, vertex.position.y, vertex.position.z };
	return vertex;
}

// ===== 描画ヘルパー関数 =====

void Object3d::BindVertexBuffer()
{
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->IASetVertexBuffers(
		0, 1, &vertexBufferView_);
}

void Object3d::SetMaterialCBV()
{
	Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(
		0, materialResource_->GetGPUVirtualAddress());
}

void Object3d::SetLightingCBVs()
{
	auto* commandList = Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList();
	
	// ディレクショナルライト (slot 3)
	commandList->SetGraphicsRootConstantBufferView(
		3, directionalLightResource_->GetGPUVirtualAddress());
	
	// カメラ (slot 4)
	commandList->SetGraphicsRootConstantBufferView(
		4, cameraResource_->GetGPUVirtualAddress());
	
	// ポイントライト (slot 5)
	commandList->SetGraphicsRootConstantBufferView(
		5, pointLightResource_->GetGPUVirtualAddress());
	
	// スポットライト (slot 6)
	commandList->SetGraphicsRootConstantBufferView(
		6, spotLightResource_->GetGPUVirtualAddress());
}

void Object3d::SetTextureSRVs()
{
	auto* commandList = Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList();
	
	// モデルテクスチャ (slot 2)
	commandList->SetGraphicsRootDescriptorTable(
		2, modelData_.material.gpuHandle);
	
	// スカイボックス (slot 7)
	if (skyboxGpuHandle_.ptr != 0) {
		commandList->SetGraphicsRootDescriptorTable(
			7, skyboxGpuHandle_);
	}
}
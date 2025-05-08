#include "ParticleManager.h"
#include <Logger.h>
#include <TextureManager.h>
#include <MakeIdentity4x4.h>
#include <MakeAffineMatrix.h>
#include <Inverse.h>
#include <MakeScaleMatrix.h>
#include <MakeTranslateMatrix.h>
#include <Material.h>
#include <imgui.h>
#include <MakeRotateXYZMatrix.h>
#include <iostream>
#include <numbers>

using namespace Logger;

//ParticleManager* ParticleManager::instance = nullptr;

ParticleManager* ParticleManager::GetInstance()
{
	static ParticleManager instance;
	return &instance;
}

void ParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, Camera* camera)
{
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
	camera_ = camera;

	// ランダムエンジンの初期化
	randomEngine.seed(seedGeneral());

	// 加速フィールドの初期化
	accelerationField.acceleration = { 15.0f, 0.0f, 0.0f };
	accelerationField.area.min = { -10.0f, -10.0f, -30.0f };
	accelerationField.area.max = { 10.0f, 10.0f, 30.0f };



	// PSOの初期化
	CreatePSO();

	// 頂点データの初期化
	CreateVertexData();
	//CreateRingVertexData();
	//CreateCylinderVertexData();

	// マテリアルデータの初期化
	CreateMaterialData();
}

void ParticleManager::Update()
{
	// ビュー行列とプロジェクション行列をカメラから取得
	Matrix4x4 cameraMatrix = MakeAffineMatrix::MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, camera_->GetRotate(), camera_->GetTranslate());
	Matrix4x4 viewMatrix = Inverse::Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = camera_->GetProjectionMatrix();
	Matrix4x4 viewProjectionMatrix = Multiply::Multiply(viewMatrix, projectionMatrix);

	// 最初からこっちを向いているモデルはこれでいいが、反対側を向いているモデルは180度回転させなければならない
	Matrix4x4 backToFrontMatrix = MakeIdentity4x4::MakeIdentity4x4();

	// ビルボード行列を初期化
	Matrix4x4 scaleMatrix{};


	Matrix4x4 rotateMatrix{};


	Matrix4x4 translateMatrix{};

	Matrix4x4 uvTransformMatrix = MakeScaleMatrix::MakeScaleMatrix(uvTransform.scale);
	uvTransformMatrix = Multiply::Multiply(uvTransformMatrix, MakeRotateZMatrix::MakeRotateZMatrix(uvTransform.rotate.z));
	uvTransformMatrix = Multiply::Multiply(uvTransformMatrix, MakeTranslateMatrix::MakeTranslateMatrix(uvTransform.translate));
	materialData_->uvTransform = uvTransformMatrix;

	uvTransform.translate.x += 0.0001f;

	Matrix4x4 billboardMatrix = Multiply::Multiply(backToFrontMatrix, cameraMatrix);
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;
	//billboardMatrix = MakeIdentity4x4();
	// パーティクルグループごとに更新処理
	for (auto& group : particleGroups)
	{
		group.second.numParticles = 0; // 生存パーティクル数をリセット

		for (std::list<Particle>::iterator particleIterator = group.second.particles.begin(); particleIterator != group.second.particles.end();)
		{
			// 生存時間を超えたパーティクルは削除
			if ((*particleIterator).lifeTime <= (*particleIterator).currentTime)
			{
				particleIterator = group.second.particles.erase(particleIterator);
				continue;
			}

			// 最大インスタンス数を超えない場合のみ更新
			if (group.second.numParticles < kNumMaxInstance)
			{
				// 速度を適用して位置を更新
				(*particleIterator).transform.translate += (*particleIterator).velocity * kDeltaTime;
				(*particleIterator).currentTime += kDeltaTime; // 経過時間を加算

				// スケール、回転、平行移動を利用してワールド行列を作成
				Matrix4x4 worldMatrix = MakeAffineMatrix::MakeAffineMatrix((*particleIterator).transform.scale, (*particleIterator).transform.rotate, (*particleIterator).transform.translate);
				scaleMatrix = MakeScaleMatrix::MakeScaleMatrix((*particleIterator).transform.scale);

				translateMatrix = MakeTranslateMatrix::MakeTranslateMatrix((*particleIterator).transform.translate);

				// ビルボードを使うかどうか
				if (useBillboard)
				{
					worldMatrix = scaleMatrix * billboardMatrix * translateMatrix;
				} else {
					rotateMatrix = MakeRotateXYZMatrix::MakeRotateXYZMatrix((*particleIterator).transform.rotate);
					worldMatrix = scaleMatrix * rotateMatrix * translateMatrix;
				}

				// ワールド・ビュー・プロジェクション行列を計算
				Matrix4x4 worldViewProjectionMatrix = Multiply::Multiply(worldMatrix, viewProjectionMatrix);

				// インスタンシング用データを設定
				group.second.instanceData[group.second.numParticles].WVP = worldViewProjectionMatrix;
				group.second.instanceData[group.second.numParticles].World = worldMatrix;

				// 色とアルファ値を設定
				float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifeTime);
				group.second.instanceData[group.second.numParticles].color = (*particleIterator).color;
				group.second.instanceData[group.second.numParticles].color.w = alpha;

				if (isWind)
				{
					// フィールドの範囲内のパーティクルには加速度を適用する
						// 各パーティクルに最適な風を適用
					for (const auto& zone : windZones) {
						if (IsCollision(accelerationField.area, (*particleIterator).transform.translate)) {
							(*particleIterator).velocity.x += zone.strength.x;
							(*particleIterator).velocity.y += zone.strength.y;
							(*particleIterator).velocity.z += zone.strength.z;
						}
					}
				}

				// 生存パーティクル数をカウント
				++group.second.numParticles;
			}

			// 次のパーティクルに進む
			++particleIterator;
		}
	}
}

void ParticleManager::Draw()
{
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	// ルートシグネチャを設定
	commandList->SetGraphicsRootSignature(rootSignature_.Get());

	// パイプラインステートオブジェクト (PSO) を設定
	commandList->SetPipelineState(graphicsPipelineState.Get());

	//プリミティブトポロジを設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// VBVを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	// すべてのパーティクルグループについて処理する
	for (auto& group : particleGroups)
	{
		if (group.second.numParticles == 0) continue;

		// マテリアルCBVを設定
		commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

		// テクスチャのSRVのデスクリプタテーブルを設定
		commandList->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(group.second.srvIndex));

		// インスタンシングデータのSRVのデスクリプタテーブルを設定
		commandList->SetGraphicsRootDescriptorTable(2, group.second.materialData.gpuHandle);

		// インスタンシング描画

		commandList->DrawInstanced(UINT(modelData.vertices.size()), group.second.numParticles, 0, 0);

		// インスタンス数をリセット
		group.second.numParticles = 0;
	}
}

void ParticleManager::Finalize()
{
	// particleGroups 内のリソースを解放
	for (auto& [key, group] : particleGroups)
	{
		group.instanceBuffer.Reset(); // ComPtr の解放
		group.instanceData = nullptr;  // ポインタを無効化
	}
	particleGroups.clear();
}

void ParticleManager::CreateParticleGroup(const std::string& name, const std::string textureFilePath)
{
	// パーティクルグループが既に存在するか確認
	if (particleGroups.find(name) != particleGroups.end())
	{
		// エラーメッセージを出力して処理を中断
		std::cerr << "Error: Particle group '" << name << "' already exists!" << std::endl;
		return;
	}
	// 新たな空のパーティクルグループを作成し、コンテナに登録
	ParticleGroup group{};
	group.materialData.textureFilePath = textureFilePath;
	group.materialData.gpuHandle = TextureManager::GetInstance()->GetSrvHandleGPU(textureFilePath);

	// インスタンスバッファ作成
	group.instanceBuffer = CreateBufferResource(dxCommon_->GetDevice().Get(), sizeof(ParticleForGPU) * kNumMaxInstance);
	group.instanceBuffer->Map(0, nullptr, reinterpret_cast<void**>(&group.instanceData));

	// 初期化
	for (uint32_t i = 0; i < kNumMaxInstance; ++i)
	{
		group.instanceData[i].WVP = MakeIdentity4x4::MakeIdentity4x4();
		group.instanceData[i].World = MakeIdentity4x4::MakeIdentity4x4();
	}

	// インスタンシング用SRVの生成
	group.srvIndex = srvManager_->Allocate();
	srvManager_->CreateSRVforStructuredBuffer(group.srvIndex, group.instanceBuffer.Get(), kNumMaxInstance, sizeof(ParticleForGPU));

	particleGroups.emplace(name, group);
}

Microsoft::WRL::ComPtr<ID3D12Resource> ParticleManager::CreateBufferResource(ID3D12Device* device, size_t sizeInBytes)
{
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
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

void ParticleManager::Emit(const std::string name, const Vector3& position, uint32_t count)
{
	// パーティクルグループが存在するかどうか
	assert(particleGroups.find(name) != particleGroups.end() && "Particle Group is not found");

	// パーティクルグループを取得
	ParticleGroup& particleGroup = particleGroups[name];

	// 最大数に達している場合
	if (particleGroup.particles.size() >= count) return;

	// パーティクルの生成
	for (uint32_t index = 0; index < count; ++index)
	{
		// パーティクルの生成と追加
		particleGroup.particles.push_back(MakeNewPlaneParticle(randomEngine, position));
	}
}

ParticleManager::Particle ParticleManager::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate)
{
	Particle particle;

	// 一様分布生成期を使って乱数を生成
	std::uniform_real_distribution<float> distribution(-5.0, 5.0f);
	std::uniform_real_distribution<float> distColor(0.0, 1.0f);
	std::uniform_real_distribution<float> distTime(1.0, 3.0f);

	// 位置と速度を[-1, 1]でランダムに初期化
	particle.transform.scale = { 1.0f, 1.0f, 1.0f };
	particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
	particle.transform.translate = translate;

	// 色を[0, 1]でランダムに初期化
	particle.color = { distColor(randomEngine), distColor(randomEngine), distColor(randomEngine), 1.0f };

	// パーティクル生成時にランダムに1秒～3秒の間生存
	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0;
	particle.velocity = { distribution(randomEngine),distribution(randomEngine),distribution(randomEngine) };

	return particle;
}

ParticleManager::Particle ParticleManager::MakeNewPlaneParticle(std::mt19937& randomEngine, const Vector3& translate)
{
	Particle particle;

	// 一様分布生成期を使って乱数を生成
	std::uniform_real_distribution<float> distRotate(-std::numbers::pi_v<float>, std::numbers::pi_v<float>);
	std::uniform_real_distribution<float> distScale(0.4f, 1.5f);
	// 位置と速度を[-1, 1]でランダムに初期化
	particle.transform.scale = { 0.05f, distScale(randomEngine), 1.0f };
	particle.transform.rotate = { 0.0f, 0.0f, distRotate(randomEngine) };
	particle.transform.translate = translate;


	particle.color = { 1.0f,1.0f,1.0f, 1.0f };

	// パーティクル生成時にランダムに1秒～3秒の間生存
	particle.lifeTime = 1.0f;
	particle.currentTime = 0;
	particle.velocity = { 0,0,0 };

	return particle;
}

ParticleManager::Particle ParticleManager::MakeNewRingParticle(std::mt19937& randomEngine, const Vector3& translate)
{
	Particle particle;

	std::uniform_real_distribution<float> distRotate(-std::numbers::pi_v<float>, std::numbers::pi_v<float>);
	// 位置と速度を[-1, 1]でランダムに初期化
	particle.transform.scale = { 1.0f, 1.0f, 1.0f };
	particle.transform.rotate = { 1.0f, 1.0f, distRotate(randomEngine) };
	particle.transform.translate = translate;

	particle.color = { 1.0f,1.0f,1.0f, 1.0f };

	// パーティクル生成時にランダムに1秒～3秒の間生存
	particle.lifeTime = 1.0f;
	particle.currentTime = 0;
	particle.velocity = { 0,0,0 };

	return particle;
}

ParticleManager::Particle ParticleManager::MakeNewCylinderParticle(std::mt19937& randomEngine, const Vector3& translate)
{
	Particle particle;

	std::uniform_real_distribution<float> distRotate(-std::numbers::pi_v<float>, std::numbers::pi_v<float>);
	// 位置と速度を[-1, 1]でランダムに初期化
	particle.transform.scale = { 1.0f, 1.0f, 1.0f };
	particle.transform.rotate = { -0.2f, 0.0f, 0.0f };
	particle.transform.translate = translate;

	particle.color = { 0.0f,0.0f,1.0f, 1.0f };

	// パーティクル生成時にランダムに1秒～3秒の間生存
	particle.lifeTime = 1000.0f;
	particle.currentTime = 0;
	particle.velocity = { 0,0,0 };


	return particle;
}

void ParticleManager::CreateRingVertexData()
{
	const uint32_t kRingDivide = 32;
	const float kOuterRadius = 1.0f;
	const float kInnerRadius = 0.2f;
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kRingDivide);

	for (uint32_t index = 0; index < kRingDivide; ++index)
	{
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);
		float u = float(index) / float(kRingDivide);
		float uNext = float(index + 1) / float(kRingDivide);
		// 頂点データを作成
		modelData.vertices.push_back({ .position = {-sin * kInnerRadius,cos * kInnerRadius,0.0f,1.0f},.texcoord = {u, 1.0f},.normal = {0.0f, 0.0f, 1.0f} });	// 内周1
		modelData.vertices.push_back({ .position = {-sinNext * kInnerRadius,cosNext * kInnerRadius,0.0f,1.0f},.texcoord = {uNext, 1.0f},.normal = {0.0f, 0.0f, 1.0f} });	// 内周2
		modelData.vertices.push_back({ .position = {-sinNext * kOuterRadius,cosNext * kOuterRadius,0.0f,1.0f},.texcoord = {uNext, 0.0f},.normal = {0.0f, 0.0f, 1.0f} });	// 外周2
		modelData.vertices.push_back({ .position = {-sin * kOuterRadius,cos * kOuterRadius,0.0f,1.0f},.texcoord = {u, 0.0f},.normal = {0.0f, 0.0f, 1.0f} });	// 外周1
		modelData.vertices.push_back({ .position = {-sin * kInnerRadius,cos * kInnerRadius,0.0f,1.0f},.texcoord = {u, 1.0f},.normal = {0.0f, 0.0f, 1.0f} });	// 内周1
		modelData.vertices.push_back({ .position = {-sinNext * kOuterRadius,cosNext * kOuterRadius,0.0f,1.0f},.texcoord = {uNext, 0.0f},.normal = {0.0f, 0.0f, 1.0f} });	// 外周2

	}

	// リソースの作成
	vertexResource = CreateBufferResource(dxCommon_->GetDevice().Get(), sizeof(VertexData) * modelData.vertices.size());

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();

	// 使用するリソースのサイズは頂点のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());

	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// 頂点データをリソースにコピー
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
}

void ParticleManager::CreateCylinderVertexData()
{
	const uint32_t kCylinderDivide = 32;
	const float kTopRadius = 1.0f;
	const float kBottomRadius = 1.0f;
	const float kHeight = 3.0f;
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kCylinderDivide);

	for (uint32_t index = 0; index < kCylinderDivide; ++index)
	{
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);
		float u = float(index) / float(kCylinderDivide);
		float uNext = float(index + 1) / float(kCylinderDivide);
 
		// 頂点データを作成
		modelData.vertices.push_back({ .position = {-sin * kTopRadius,kHeight,cos * kBottomRadius,1.0f},.texcoord = {u, 1.0f},.normal = {-sin, 0.0f, cos} });
		modelData.vertices.push_back({ .position = {-sinNext * kTopRadius,kHeight,cosNext * kBottomRadius,1.0f},.texcoord = {uNext, 1.0f},.normal = {-sinNext, 0.0f, cosNext} });
		modelData.vertices.push_back({ .position = {-sin * kBottomRadius,0.0f,cos * kBottomRadius,1.0f},.texcoord = {u, 0.0f},.normal = {-sin, 0.0f, cos} });
		modelData.vertices.push_back({ .position = {-sin * kBottomRadius,0.0f,cos * kBottomRadius,1.0f},.texcoord = {u, 0.0f},.normal = {-sin, 0.0f, cos} });
		modelData.vertices.push_back({ .position = {-sinNext * kTopRadius,kHeight,cosNext * kTopRadius,1.0f},.texcoord = {uNext, 1.0f},.normal = {-sinNext, 0.0f, cosNext} });
		modelData.vertices.push_back({ .position = {-sinNext * kBottomRadius,0.0f,cosNext * kBottomRadius,1.0f},.texcoord = {uNext, 0.0f},.normal = {-sinNext, 0.0f, cosNext} });
	}
	// リソースの作成
	vertexResource = CreateBufferResource(dxCommon_->GetDevice().Get(), sizeof(VertexData) * modelData.vertices.size());

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();

	// 使用するリソースのサイズは頂点のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());

	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// 頂点データをリソースにコピー
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
}

void ParticleManager::CreateVertexData()
{
	modelData.vertices.push_back({ .position = {-1.0f,1.0f,0.0f,1.0f},.texcoord = {0.0f,0.0f},.normal = {0.0f,0.0f,1.0f} });// 左上
	modelData.vertices.push_back({ .position = {1.0f,1.0f,0.0f,1.0f},.texcoord = {1.0f,0.0f},.normal = {0.0f,0.0f,1.0f} });// 右上
	modelData.vertices.push_back({ .position = {-1.0f,-1.0f,0.0f,1.0f},.texcoord = {0.0f,1.0f},.normal = {0.0f,0.0f,1.0f} });// 左下
	modelData.vertices.push_back({ .position = {-1.0f,-1.0f,0.0f,1.0f},.texcoord = {0.0f,1.0f},.normal = {0.0f,0.0f,1.0f} });// 左下
	modelData.vertices.push_back({ .position = {1.0f,1.0f,0.0f,1.0f},.texcoord = {1.0f,0.0f},.normal = {0.0f,0.0f,1.0f} });// 右上
	modelData.vertices.push_back({ .position = {1.0f,-1.0f,0.0f,1.0f},.texcoord = {1.0f,1.0f},.normal = {0.0f,0.0f,1.0f} });// 右下
	// リソースの作成
	vertexResource = CreateBufferResource(dxCommon_->GetDevice().Get(), sizeof(VertexData) * modelData.vertices.size());

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();

	// 使用するリソースのサイズは頂点のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());

	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// 頂点データをリソースにコピー
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
}

void ParticleManager::CreateMaterialData()
{
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResource = CreateBufferResource(dxCommon_->GetDevice().Get(), sizeof(Material));

	// 書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	// 今回は白を書き込んでみる
	// RGBA
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	materialData_->enableLighting = false;

	// 単位行列で初期化
	materialData_->uvTransform = MakeIdentity4x4::MakeIdentity4x4();
}

void ParticleManager::DrawImGui()
{
	ImGui::Begin("ParticleManager");
	if (ImGui::Button(useBillboard ? "Disable Billboard" : "Enable Billboard"))
	{
		// ボタンが押されたらuseBillboardの値を切り替える
		useBillboard = !useBillboard;
	}
	ImGui::DragFloat2("UVTranslate", &uvTransform.translate.x, 0.01f, -10.0f, 10.0f);
	ImGui::DragFloat2("UVScale", &uvTransform.scale.x, 0.01f, -10.0f, 10.0f);
	ImGui::SliderAngle("UVRotate", &uvTransform.rotate.z);

	/*if (ImGui::Button(isWind ? "Disable Wind" : "Enable Wind"))
	{
		isWind = !isWind;
	}*/

	ImGui::End(); // ウィンドウの終了
}

void ParticleManager::CreateRootSignature()
{
	/*------RootSignature作成------*/
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// テクスチャのディスクリプタヒープを作成する
	D3D12_DESCRIPTOR_RANGE rangeTexture[1]{};
	rangeTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeTexture[0].NumDescriptors = 1;
	rangeTexture[0].BaseShaderRegister = 0;
	rangeTexture[0].RegisterSpace = 0;

	D3D12_ROOT_PARAMETER rootParameters[3]{};
	// CBVを使う
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	// レジスタ番号0とバインド
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 0;
	// pixelShaderで使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// DescriptorTableを使う
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	// vertexShaderで使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	// レジスタ番号0とバインド
	rootParameters[1].Descriptor.ShaderRegister = 0;
	rootParameters[1].DescriptorTable.pDescriptorRanges = rangeTexture;
	// Tableで利用する数
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(rangeTexture);

	// DescriptorTableを使う
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	// PixelShaderで使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	// Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.pDescriptorRanges = rangeTexture;
	// Tableで利用する数
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(rangeTexture);


	descriptionRootSignature.NumParameters = _countof(rootParameters);
	descriptionRootSignature.pParameters = rootParameters;


	//////////////////////////
	// Samplerの設定
	//////////////////////////
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	// バイリニアフィルタ
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	// 0~1の範囲外をリピート
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	// 比較しない
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	// ありったけのMipmapを使う
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	// レジスタ番号0を使う
	staticSamplers[0].ShaderRegister = 0;
	// PixelShaderで使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);
	descriptionRootSignature.pStaticSamplers = staticSamplers;

	// シリアライズされたデータを格納するためのバッファ
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	// エラーメッセージを格納するためのバッファ
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	// バイナリを元に生成
	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
}

void ParticleManager::CreatePSO()
{

	CreateRootSignature();
	/// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	/// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// すべての色要素を書きこむ
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	// Blendのフラグ

	enum BlendMode {
		kBlendModeNone,
		kBlendModeAdd,
		kBlendModeSubtract,
		kBlendModeMultiply,
		kBlendModeScreen
	};
	int mode = kBlendModeAdd;

	switch (mode) {
		// ブレンド無し
	case kBlendModeNone:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		break;
		// 加算合成
	case kBlendModeAdd:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
		// 減算合成
	case kBlendModeSubtract:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_SUBTRACT;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
		// 乗算合成
	case kBlendModeMultiply:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
		break;
		// スクリーン合成
	case kBlendModeScreen:
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	}



	/// RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	/// VertexShader
	// shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon_->CompileShader(L"Particle.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	/// PixelShader
	// shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon_->CompileShader(L"Particle.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	// 書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	// 比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	////////////////////////
	/// PSOを生成する
	////////////////////////
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	// RootSignature
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
	// InputLayout
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	// Blendstate
	graphicsPipelineStateDesc.BlendState = blendDesc;
	// RasterizerState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	// VertexShader
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize() };
	// PixelShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() };
	// DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// 利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// どのように画面に打ち込むのかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// 実際に生成
	HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

}

bool ParticleManager::IsCollision(const AABB& aabb, const Vector3& point)
{
	if ((aabb.min.x <= point.x && aabb.max.x >= point.x) &&
		(aabb.min.y <= point.y && aabb.max.y >= point.y) &&
		(aabb.min.z <= point.z && aabb.max.z >= point.z)) {
		return true;
	} else {
		return false;
	}
}

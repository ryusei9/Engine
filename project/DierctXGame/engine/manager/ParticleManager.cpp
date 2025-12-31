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
#include <Lerp.h>
#include "DirectXCommon.h"

using namespace Logger;
using namespace ParticleManagerConstants;

namespace {
	// 加速度フィールドのデフォルト値
	constexpr Vector3 kDefaultAcceleration = { 15.0f, 0.0f, 0.0f };
	constexpr Vector3 kDefaultAccelFieldMin = { -10.0f, -10.0f, -30.0f };
	constexpr Vector3 kDefaultAccelFieldMax = { 10.0f, 10.0f, 30.0f };
	
	// ビルボード行列のオフセット（配列インデックスなのでsize_t型）
	constexpr size_t kBillboardMatrixOffsetIndex = 3;
	constexpr float kBillboardMatrixOffsetValue = 0.0f;
	
	// 爆発パーティクルの初期値
	constexpr Vector3 kExplosionCenterScale = { 0.05f, 0.05f, 0.05f };
	constexpr Vector3 kExplosionCenterVelocity = { 0.0f, 0.0f, 0.0f };
	constexpr Vector3 kExplosionCenterRotation = { 0.0f, 0.0f, 0.0f };
	
	// 頂点データのデフォルト値
	constexpr float kQuadVertexSize = 1.0f;
	constexpr float kVertexDepth = 0.0f;
	constexpr float kVertexW = 1.0f;
	constexpr Vector3 kVertexNormal = { 0.0f, 0.0f, 1.0f };
	
	// プレーンパーティクルのスケール範囲
	constexpr float kPlaneScaleX = 0.05f;
	constexpr float kPlaneScaleMin = 0.4f;
	constexpr float kPlaneScaleMax = 1.5f;
	constexpr float kPlaneScaleZ = 1.0f;
	constexpr float kPlaneLifeTime = 1.0f;
	
	// リングパーティクルの回転
	constexpr float kRingRotationX = 1.0f;
	constexpr float kRingRotationY = 1.0f;
	constexpr float kRingLifeTime = 1.0f;
}

ParticleManager* ParticleManager::GetInstance()
{
	static ParticleManager instance;
	return &instance;
}

void ParticleManager::Initialize(SrvManager* srvManager, Camera* camera)
{
	dxCommon_ = DirectXCommon::GetInstance();
	srvManager_ = srvManager;
	camera_ = camera;

	// ランダムエンジンの初期化
	randomEngine_.seed(seedGeneral_());

	// 加速フィールドの初期化
	accelerationField_.acceleration = kDefaultAcceleration;
	accelerationField_.area.min = kDefaultAccelFieldMin;
	accelerationField_.area.max = kDefaultAccelFieldMax;

	// PSOの初期化
	CreatePSO();

	// マテリアルデータの初期化
	CreateMaterialData();
}

void ParticleManager::Update()
{
	// ビュー行列とプロジェクション行列をカメラから取得
	Matrix4x4 cameraMatrix = MakeAffineMatrix::MakeAffineMatrix(
		{ 1.0f, 1.0f, 1.0f }, 
		camera_->GetRotate(), 
		camera_->GetTranslate());
	Matrix4x4 viewMatrix = Inverse::Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = camera_->GetProjectionMatrix();
	Matrix4x4 viewProjectionMatrix = Multiply::Multiply(viewMatrix, projectionMatrix);

	// UV変換行列の計算
	Matrix4x4 uvTransformMatrix = MakeScaleMatrix::MakeScaleMatrix(uvTransform_.scale);
	uvTransformMatrix = Multiply::Multiply(uvTransformMatrix, MakeRotateZMatrix::MakeRotateZMatrix(uvTransform_.rotate.z));
	uvTransformMatrix = Multiply::Multiply(uvTransformMatrix, MakeTranslateMatrix::MakeTranslateMatrix(uvTransform_.translate));
	materialData_->uvTransform = uvTransformMatrix;

	// ビルボード行列の計算
	Matrix4x4 backToFrontMatrix = MakeIdentity4x4::MakeIdentity4x4();
	Matrix4x4 billboardMatrix = Multiply::Multiply(backToFrontMatrix, cameraMatrix);
	billboardMatrix.m[kBillboardMatrixOffsetIndex][0] = kBillboardMatrixOffsetValue;
	billboardMatrix.m[kBillboardMatrixOffsetIndex][1] = kBillboardMatrixOffsetValue;
	billboardMatrix.m[kBillboardMatrixOffsetIndex][2] = kBillboardMatrixOffsetValue;

	// パーティクルグループごとに更新処理
	for (auto& group : particleGroups_)
	{
		ParticleManager::UpdateParticleGroup(group.second, billboardMatrix, viewProjectionMatrix);
	}
}

void ParticleManager::UpdateParticleGroup(
	ParticleGroup& group, 
	const Matrix4x4& billboardMatrix, 
	const Matrix4x4& viewProjectionMatrix)
{
	group.numParticles = 0; // 生存パーティクル数をリセット

	for (auto particleIterator = group.particles.begin(); particleIterator != group.particles.end();)
	{
		// 生存時間を超えたパーティクルは削除
		if (particleIterator->lifeTime <= particleIterator->currentTime)
		{
			particleIterator = group.particles.erase(particleIterator);
			continue;
		}

		// 最大インスタンス数を超えない場合のみ更新
		if (group.numParticles < kMaxInstanceCount)
		{
			UpdateParticle(*particleIterator, group, billboardMatrix, viewProjectionMatrix);
			++group.numParticles;
		}

		++particleIterator;
	}
}

void ParticleManager::UpdateParticle(
	Particle& particle,
	ParticleGroup& group,
	const Matrix4x4& billboardMatrix,
	const Matrix4x4& viewProjectionMatrix)
{
	// 速度を適用して位置を更新
	particle.transform.translate += particle.velocity * kDeltaTime;
	particle.currentTime += kDeltaTime;

	// ワールド行列の計算
	Matrix4x4 worldMatrix = CalculateWorldMatrix(particle, billboardMatrix);

	// ワールド・ビュー・プロジェクション行列を計算
	Matrix4x4 worldViewProjectionMatrix = Multiply::Multiply(worldMatrix, viewProjectionMatrix);

	// インスタンシング用データを設定
	group.instanceData[group.numParticles].WVP = worldViewProjectionMatrix;
	group.instanceData[group.numParticles].World = worldMatrix;

	// 色とアルファ値を設定
	float alpha = 1.0f - (particle.currentTime / particle.lifeTime);
	group.instanceData[group.numParticles].color = particle.color;
	group.instanceData[group.numParticles].color.w = alpha;

	// 風の適用
	if (isWind_)
	{
		ApplyWind(particle);
	}

	// 爆発パーティクルの更新
	if (particle.isExplosion)
	{
		UpdateExplosionParticle(particle);
	}

	// パーティクルタイプ別の処理
	if (particleType_ == ParticleType::Cylinder)
	{
		uvTransform_.translate.x += kCylinderUVStep;
	}
}

Matrix4x4 ParticleManager::CalculateWorldMatrix(
	const Particle& particle,
	const Matrix4x4& billboardMatrix) const
{
	Matrix4x4 scaleMatrix = MakeScaleMatrix::MakeScaleMatrix(particle.transform.scale);
	Matrix4x4 translateMatrix = MakeTranslateMatrix::MakeTranslateMatrix(particle.transform.translate);

	if (useBillboard_)
	{
		return scaleMatrix * billboardMatrix * translateMatrix;
	}
	else
	{
		Matrix4x4 rotateMatrix = MakeRotateXYZMatrix::MakeRotateXYZMatrix(particle.transform.rotate);
		return scaleMatrix * rotateMatrix * translateMatrix;
	}
}

void ParticleManager::ApplyWind(Particle& particle)
{
	for (const auto& zone : windZones_)
	{
		if (IsCollision(accelerationField_.area, particle.transform.translate))
		{
			particle.velocity += zone.strength;
		}
	}
}

void ParticleManager::Draw()
{
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	ID3D12DescriptorHeap* heaps[] = { srvManager_->GetDescriptorHeap() };
	commandList->SetDescriptorHeaps(1, heaps);

	// ルートシグネチャを設定
	commandList->SetGraphicsRootSignature(rootSignature_.Get());

	// パイプラインステートオブジェクト (PSO) を設定
	commandList->SetPipelineState(graphicsPipelineState_.Get());

	// プリミティブトポロジを設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// VBVを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

	// すべてのパーティクルグループについて処理する
	for (auto& group : particleGroups_)
	{
		if (group.second.numParticles == 0) {
			continue;
		}

		// マテリアルCBVを設定
		commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

		// テクスチャのSRVのデスクリプタテーブルを設定
		commandList->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(group.second.srvIndex));

		// インスタンシングデータのSRVのデスクリプタテーブルを設定
		commandList->SetGraphicsRootDescriptorTable(2, group.second.materialData.gpuHandle);

		// インスタンシング描画
		commandList->DrawInstanced(UINT(modelData_.vertices.size()), group.second.numParticles, 0, 0);

		// インスタンス数をリセット
		group.second.numParticles = 0;
	}
}

void ParticleManager::Finalize()
{
	// particleGroups_ 内のリソースを解放
	for (auto& [key, group] : particleGroups_)
	{
		group.instanceBuffer.Reset();
		group.instanceData = nullptr;
	}
	particleGroups_.clear();
}

void ParticleManager::CreateParticleGroup(const std::string& name, const std::string textureFilePath)
{
	// パーティクルグループが既に存在するか確認
	if (particleGroups_.find(name) != particleGroups_.end())
	{
		std::cerr << "Error: Particle group '" << name << "' already exists!" << std::endl;
		return;
	}

	// 新たな空のパーティクルグループを作成
	ParticleGroup group{};
	group.materialData.textureFilePath = textureFilePath;
	group.materialData.gpuHandle = TextureManager::GetInstance()->GetSrvHandleGPU(textureFilePath);

	// インスタンスバッファ作成
	group.instanceBuffer = CreateBufferResource(
		dxCommon_->GetDevice().Get(), 
		sizeof(ParticleForGPU) * kMaxInstanceCount);
	group.instanceBuffer->Map(0, nullptr, reinterpret_cast<void**>(&group.instanceData));

	// 初期化
	for (uint32_t i = 0; i < kMaxInstanceCount; ++i)
	{
		group.instanceData[i].WVP = MakeIdentity4x4::MakeIdentity4x4();
		group.instanceData[i].World = MakeIdentity4x4::MakeIdentity4x4();
	}

	// インスタンシング用SRVの生成
	group.srvIndex = srvManager_->Allocate();
	srvManager_->CreateSRVforStructuredBuffer(
		group.srvIndex, 
		group.instanceBuffer.Get(), 
		kMaxInstanceCount, 
		sizeof(ParticleForGPU));

	particleGroups_.emplace(name, group);
}

Microsoft::WRL::ComPtr<ID3D12Resource> ParticleManager::CreateBufferResource(
	ID3D12Device* device, 
	size_t sizeInBytes)
{
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

	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	HRESULT hr = device->CreateCommittedResource(
		&uploadHeapProperties, 
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc, 
		D3D12_RESOURCE_STATE_GENERIC_READ, 
		nullptr, 
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

void ParticleManager::Emit(const std::string name, const Vector3& position, uint32_t count)
{
	assert(particleGroups_.find(name) != particleGroups_.end() && "Particle Group is not found");

	ParticleGroup& particleGroup = particleGroups_[name];

	if (particleGroup.particles.size() >= count) {
		return;
	}

	for (uint32_t index = 0; index < count; ++index)
	{
		Particle particle = CreateParticleByType(position);
		particleGroup.particles.push_back(particle);
	}
}

ParticleManager::Particle ParticleManager::CreateParticleByType(const Vector3& position)
{
	switch (particleType_)
	{
	case ParticleType::Normal:
	case ParticleType::Explosion:
		return MakeNewParticle(randomEngine_, position);
	case ParticleType::Plane:
		return MakeNewPlaneParticle(randomEngine_, position);
	case ParticleType::Ring:
		return MakeNewRingParticle(randomEngine_, position);
	case ParticleType::Cylinder:
		return MakeNewCylinderParticle(randomEngine_, position);
	default:
		return MakeNewParticle(randomEngine_, position);
	}
}

void ParticleManager::EmitExplosion(const std::string& name, const Vector3& position, uint32_t count)
{
	assert(particleGroups_.find(name) != particleGroups_.end() && "Particle Group is not found");

	ParticleGroup& group = particleGroups_[name];

	// 中心の大きなパーティクル
	{
		Particle particle;
		particle.transform.scale = kExplosionCenterScale;
		particle.transform.rotate = kExplosionCenterRotation;
		particle.transform.translate = position;
		particle.color = kExplosionColorCenter;
		particle.lifeTime = kExplosionCenterLifeTime;
		particle.currentTime = 0.0f;
		particle.velocity = kExplosionCenterVelocity;
		particle.isExplosion = true;
		particle.isSubExplosion = false;
		group.particles.push_back(particle);
	}

	// サブパーティクル
	std::uniform_real_distribution<float> distAngle(0.0f, 2.0f * float(std::numbers::pi));
	std::uniform_real_distribution<float> distRadius(kExplosionSubRadiusMin, kExplosionSubRadiusMax);
	std::uniform_real_distribution<float> distScale(kExplosionSubScaleMin, kExplosionSubScaleMax);
	std::uniform_real_distribution<float> distLife(kExplosionSubLifeTimeMin, kExplosionSubLifeTimeMax);
	std::uniform_real_distribution<float> distStartTime(0.0f, kExplosionSubStartTimeMax);

	for (uint32_t i = 0; i < count; ++i)
	{
		float angle = distAngle(randomEngine_);
		float radius = distRadius(randomEngine_);
		float x = std::cos(angle) * radius;
		float y = std::sin(angle) * radius;
		float z = (distAngle(randomEngine_) - static_cast<float>(std::numbers::pi)) * kExplosionSubZRange;

		Particle particle;
		particle.maxScale = distScale(randomEngine_);
		particle.transform.scale = { 0.0f, 0.0f, 0.0f };
		particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
		particle.transform.translate = position + Vector3{ x, y, z };
		particle.color = kExplosionColorSub;
		particle.lifeTime = distLife(randomEngine_);
		particle.currentTime = distStartTime(randomEngine_);
		particle.velocity = { 0.0f, 0.0f, 0.0f };
		particle.isExplosion = true;
		particle.isSubExplosion = true;
		group.particles.push_back(particle);
	}
}

void ParticleManager::EmitWithVelocity(
	const std::string& name, 
	const Vector3& position, 
	uint32_t count, 
	const Vector3& velocity)
{
	assert(particleGroups_.find(name) != particleGroups_.end() && "Particle Group is not found");
	
	ParticleGroup& particleGroup = particleGroups_[name];
	
	if (particleGroup.particles.size() >= count) {
		return;
	}

	for (uint32_t index = 0; index < count; ++index)
	{
		Particle particle = isSmoke_ 
			? MakeNewSmokeParticle(randomEngine_, position)
			: MakeNewThrusterParticle(randomEngine_, position);
		
		particle.velocity = velocity;
		particleGroup.particles.push_back(particle);
	}
}

ParticleManager::Particle ParticleManager::MakeNewParticle(
	std::mt19937& randomEngine, 
	const Vector3& translate)
{
	std::uniform_real_distribution<float> distribution(kParticleSpawnRangeMin, kParticleSpawnRangeMax);
	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
	std::uniform_real_distribution<float> distTime(kParticleLifeTimeMin, kParticleLifeTimeMax);

	Particle particle;
	particle.transform.scale = { kDefaultParticleScale, kDefaultParticleScale, kDefaultParticleScale };
	particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
	particle.transform.translate = translate;
	particle.color = { distColor(randomEngine), distColor(randomEngine), distColor(randomEngine), 1.0f };
	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0.0f;
	particle.velocity = { distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };

	return particle;
}

ParticleManager::Particle ParticleManager::MakeNewPlaneParticle(
	std::mt19937& randomEngine, 
	const Vector3& translate)
{
	std::uniform_real_distribution<float> distRotate(
		-std::numbers::pi_v<float>, 
		std::numbers::pi_v<float>);
	std::uniform_real_distribution<float> distScale(kPlaneScaleMin, kPlaneScaleMax);

	Particle particle;
	particle.transform.scale = { kPlaneScaleX, distScale(randomEngine), kPlaneScaleZ };
	particle.transform.rotate = { 0.0f, 0.0f, distRotate(randomEngine) };
	particle.transform.translate = translate;
	particle.color = kColorWhite;
	particle.lifeTime = kPlaneLifeTime;
	particle.currentTime = 0.0f;
	particle.velocity = { 0.0f, 0.0f, 0.0f };

	return particle;
}

ParticleManager::Particle ParticleManager::MakeNewRingParticle(
	std::mt19937& randomEngine, 
	const Vector3& translate)
{
	std::uniform_real_distribution<float> distRotate(
		-std::numbers::pi_v<float>, 
		std::numbers::pi_v<float>);

	Particle particle;
	particle.transform.scale = { kDefaultParticleScale, kDefaultParticleScale, kDefaultParticleScale };
	particle.transform.rotate = { kRingRotationX, kRingRotationY, distRotate(randomEngine) };
	particle.transform.translate = translate;
	particle.color = kColorWhite;
	particle.lifeTime = kRingLifeTime;
	particle.currentTime = 0.0f;
	particle.velocity = { 0.0f, 0.0f, 0.0f };

	return particle;
}

ParticleManager::Particle ParticleManager::MakeNewCylinderParticle(
	std::mt19937& randomEngine, 
	const Vector3& translate)
{
	(void)randomEngine; // 未使用パラメータ警告を抑制

	Particle particle;
	particle.transform.scale = { kDefaultParticleScale, kDefaultParticleScale, kDefaultParticleScale };
	particle.transform.rotate = kCylinderRotation;
	particle.transform.translate = translate;
	particle.color = kColorBlue;
	particle.lifeTime = kCylinderLifeTime;
	particle.currentTime = 0.0f;
	particle.velocity = { 0.0f, 0.0f, 0.0f };

	return particle;
}

ParticleManager::Particle ParticleManager::MakeNewThrusterParticle(
	std::mt19937& randomEngine, 
	const Vector3& translate)
{
	std::uniform_real_distribution<float> distribution(kParticleSpawnRangeMin, kParticleSpawnRangeMax);

	Particle particle;
	particle.transform.scale = { kThrusterParticleScale, kThrusterParticleScale, kThrusterParticleScale };
	particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
	particle.transform.translate = translate;
	particle.color = kColorCyan;
	particle.lifeTime = kThrusterLifeTime;
	particle.currentTime = 0.0f;
	particle.velocity = { distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };

	return particle;
}

ParticleManager::Particle ParticleManager::MakeNewSmokeParticle(
	std::mt19937& randomEngine, 
	const Vector3& translate)
{
	std::uniform_real_distribution<float> distribution(kParticleSpawnRangeMin, kParticleSpawnRangeMax);

	Particle particle;
	particle.transform.scale = { kSmokeParticleScale, kSmokeParticleScale, kSmokeParticleScale };
	particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
	particle.transform.translate = translate;
	particle.color = kColorGray;
	particle.lifeTime = kSmokeLifeTime;
	particle.currentTime = 0.0f;
	particle.velocity = { distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };

	return particle;
}

void ParticleManager::UpdateExplosionParticle(Particle& particle)
{
	float t = particle.currentTime / particle.lifeTime;

	// 色補間（段階的な色変化）
	if (t < kExplosionColorPhase1)
	{
		float f = t / kExplosionColorPhase1;
		particle.color = Lerp(kExplosionColor1, kExplosionColor2, f);
	}
	else if (t < kExplosionColorPhase2)
	{
		float f = (t - kExplosionColorPhase1) / (kExplosionColorPhase2 - kExplosionColorPhase1);
		particle.color = Lerp(kExplosionColor2, kExplosionColor3, f);
	}
	else if (t < kExplosionColorPhase3)
	{
		float f = (t - kExplosionColorPhase2) / (kExplosionColorPhase3 - kExplosionColorPhase2);
		particle.color = Lerp(kExplosionColor3, kExplosionColor4, f);
	}
	else
	{
		float f = (t - kExplosionColorPhase3) / (1.0f - kExplosionColorPhase3);
		particle.color = Lerp(kExplosionColor4, kExplosionColor5, f);
	}

	// スケール補間
	float maxScale = particle.isSubExplosion ? particle.maxScale : kExplosionCenterMaxScale;
	float scale = Lerp(particle.transform.scale.x, maxScale, t);
	particle.transform.scale = { scale, scale, scale };
}

void ParticleManager::CreateRingVertexData()
{
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kRingDivision);

	for (uint32_t index = 0; index < kRingDivision; ++index)
	{
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);
		float u = float(index) / float(kRingDivision);
		float uNext = float(index + 1) / float(kRingDivision);

		// 頂点データを作成（6頂点で四角形2つ＝リングの一部）
		modelData_.vertices.push_back({ 
			.position = {-sin * kRingInnerRadius, cos * kRingInnerRadius, kVertexDepth, kVertexW}, 
			.texcoord = {u, 1.0f}, 
			.normal = kVertexNormal 
		});
		modelData_.vertices.push_back({ 
			.position = {-sinNext * kRingInnerRadius, cosNext * kRingInnerRadius, kVertexDepth, kVertexW}, 
			.texcoord = {uNext, 1.0f}, 
			.normal = kVertexNormal 
		});
		modelData_.vertices.push_back({ 
			.position = {-sinNext * kRingOuterRadius, cosNext * kRingOuterRadius, kVertexDepth, kVertexW}, 
			.texcoord = {uNext, 0.0f}, 
			.normal = kVertexNormal 
		});
		modelData_.vertices.push_back({ 
			.position = {-sin * kRingOuterRadius, cos * kRingOuterRadius, kVertexDepth, kVertexW}, 
			.texcoord = {u, 0.0f}, 
			.normal = kVertexNormal 
		});
		modelData_.vertices.push_back({ 
			.position = {-sin * kRingInnerRadius, cos * kRingInnerRadius, kVertexDepth, kVertexW}, 
			.texcoord = {u, 1.0f}, 
			.normal = kVertexNormal 
		});
		modelData_.vertices.push_back({ 
			.position = {-sinNext * kRingOuterRadius, cosNext * kRingOuterRadius, kVertexDepth, kVertexW}, 
			.texcoord = {uNext, 0.0f}, 
			.normal = kVertexNormal 
		});
	}

	CreateVertexBuffer();
}

void ParticleManager::CreateCylinderVertexData()
{
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kCylinderDivision);

	for (uint32_t index = 0; index < kCylinderDivision; ++index)
	{
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);
		float u = float(index) / float(kCylinderDivision);
		float uNext = float(index + 1) / float(kCylinderDivision);

		// 円柱の側面頂点データ（6頂点で四角形2つ）
		modelData_.vertices.push_back({ 
			.position = {-sin * kCylinderTopRadius, kCylinderHeight, cos * kCylinderBottomRadius, kVertexW}, 
			.texcoord = {u, 1.0f}, 
			.normal = {-sin, 0.0f, cos} 
		});
		modelData_.vertices.push_back({ 
			.position = {-sinNext * kCylinderTopRadius, kCylinderHeight, cosNext * kCylinderBottomRadius, kVertexW}, 
			.texcoord = {uNext, 1.0f}, 
			.normal = {-sinNext, 0.0f, cosNext} 
		});
		modelData_.vertices.push_back({ 
			.position = {-sin * kCylinderBottomRadius, 0.0f, cos * kCylinderBottomRadius, kVertexW}, 
			.texcoord = {u, 0.0f}, 
			.normal = {-sin, 0.0f, cos} 
		});
		modelData_.vertices.push_back({ 
			.position = {-sin * kCylinderBottomRadius, 0.0f, cos * kCylinderBottomRadius, kVertexW}, 
			.texcoord = {u, 0.0f}, 
			.normal = {-sin, 0.0f, cos} 
		});
		modelData_.vertices.push_back({ 
			.position = {-sinNext * kCylinderTopRadius, kCylinderHeight, cosNext * kCylinderTopRadius, kVertexW}, 
			.texcoord = {uNext, 1.0f}, 
			.normal = {-sinNext, 0.0f, cosNext} 
		});
		modelData_.vertices.push_back({ 
			.position = {-sinNext * kCylinderBottomRadius, 0.0f, cosNext * kCylinderBottomRadius, kVertexW}, 
			.texcoord = {uNext, 0.0f}, 
			.normal = {-sinNext, 0.0f, cosNext} 
		});
	}

	CreateVertexBuffer();
}

void ParticleManager::CreateVertexData()
{
	// 四角形の頂点データ（6頂点で三角形2つ）
	modelData_.vertices.push_back({ 
		.position = {-kQuadVertexSize, kQuadVertexSize, kVertexDepth, kVertexW}, 
		.texcoord = {0.0f, 0.0f}, 
		.normal = kVertexNormal 
	});
	modelData_.vertices.push_back({ 
		.position = {kQuadVertexSize, kQuadVertexSize, kVertexDepth, kVertexW}, 
		.texcoord = {1.0f, 0.0f}, 
		.normal = kVertexNormal 
	});
	modelData_.vertices.push_back({ 
		.position = {-kQuadVertexSize, -kQuadVertexSize, kVertexDepth, kVertexW}, 
		.texcoord = {0.0f, 1.0f}, 
		.normal = kVertexNormal 
	});
	modelData_.vertices.push_back({ 
		.position = {-kQuadVertexSize, -kQuadVertexSize, kVertexDepth, kVertexW}, 
		.texcoord = {0.0f, 1.0f}, 
		.normal = kVertexNormal 
	});
	modelData_.vertices.push_back({ 
		.position = {kQuadVertexSize, kQuadVertexSize, kVertexDepth, kVertexW}, 
		.texcoord = {1.0f, 0.0f}, 
		.normal = kVertexNormal 
	});
	modelData_.vertices.push_back({ 
		.position = {kQuadVertexSize, -kQuadVertexSize, kVertexDepth, kVertexW}, 
		.texcoord = {1.0f, 1.0f}, 
		.normal = kVertexNormal 
	});

	CreateVertexBuffer();
}

void ParticleManager::CreateVertexBuffer()
{
	// リソースの作成
	vertexResource_ = CreateBufferResource(
		dxCommon_->GetDevice().Get(), 
		sizeof(VertexData) * modelData_.vertices.size());

	// 頂点バッファビューの設定
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 頂点データをマップしてコピー
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
}

void ParticleManager::CreateMaterialData()
{
	materialResource_ = CreateBufferResource(
		dxCommon_->GetDevice().Get(), 
		sizeof(Material));

	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	materialData_->color = kColorWhite;
	materialData_->enableLighting = false;
	materialData_->uvTransform = MakeIdentity4x4::MakeIdentity4x4();
}

void ParticleManager::DrawImGui()
{
#ifdef USE_IMGUI
	ImGui::Begin("ParticleManager");
	
	if (ImGui::Button(useBillboard_ ? "Disable Billboard" : "Enable Billboard"))
	{
		useBillboard_ = !useBillboard_;
	}
	
	ImGui::DragFloat2("UVTranslate", &uvTransform_.translate.x, 0.01f, -10.0f, 10.0f);
	ImGui::DragFloat2("UVScale", &uvTransform_.scale.x, 0.01f, -10.0f, 10.0f);
	ImGui::SliderAngle("UVRotate", &uvTransform_.rotate.z);

	ImGui::End();
#endif
}

void ParticleManager::SetParticleType(ParticleType type)
{
	particleType_ = type;
	
	// 頂点データを切り替え
	switch (particleType_)
	{
	case ParticleType::Normal:
	case ParticleType::Plane:
	case ParticleType::Explosion:
		CreateVertexData();
		break;
	case ParticleType::Ring:
		CreateRingVertexData();
		break;
	case ParticleType::Cylinder:
		CreateCylinderVertexData();
		break;
	}
}

void ParticleManager::CreateRootSignature()
{
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE rangeTexture[1]{};
	rangeTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeTexture[0].NumDescriptors = 1;
	rangeTexture[0].BaseShaderRegister = 0;
	rangeTexture[0].RegisterSpace = 0;

	D3D12_ROOT_PARAMETER rootParameters[kRootParameterCount]{};
	
	// ルートパラメータ0: マテリアルCBV
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 0;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// ルートパラメータ1: テクスチャSRV（VertexShader用）
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;
	rootParameters[1].DescriptorTable.pDescriptorRanges = rangeTexture;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;

	// ルートパラメータ2: インスタンシングデータSRV（PixelShader用）
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = rangeTexture;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

	descriptionRootSignature.NumParameters = kRootParameterCount;
	descriptionRootSignature.pParameters = rootParameters;

	// 静的サンプラの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[kStaticSamplerCount]{};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descriptionRootSignature.NumStaticSamplers = kStaticSamplerCount;
	descriptionRootSignature.pStaticSamplers = staticSamplers;

	// ルートシグネチャのシリアライズ
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	HRESULT hr = D3D12SerializeRootSignature(
		&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, 
		&signatureBlob, 
		&errorBlob);
	
	if (FAILED(hr))
	{
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	// ルートシグネチャの生成
	hr = dxCommon_->GetDevice()->CreateRootSignature(
		0, 
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), 
		IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
}

void ParticleManager::CreatePSO()
{
	CreateRootSignature();

	// 入力レイアウト
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[kInputElementCount]{};
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
	inputLayoutDesc.NumElements = kInputElementCount;

	// ブレンドステートの設定（加算合成）
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE; // 加算合成
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	// ラスタライザステート
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// シェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon_->CompileShader(
		kVertexShaderPath, 
		kVertexShaderProfile);
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon_->CompileShader(
		kPixelShaderPath, 
		kPixelShaderProfile);
	assert(pixelShaderBlob != nullptr);

	// デプスステンシルステート
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // 深度書き込み無効
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// パイプラインステート記述子
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	graphicsPipelineStateDesc.VS = { 
		vertexShaderBlob->GetBufferPointer(), 
		vertexShaderBlob->GetBufferSize() 
	};
	graphicsPipelineStateDesc.PS = { 
		pixelShaderBlob->GetBufferPointer(), 
		pixelShaderBlob->GetBufferSize() 
	};
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// パイプラインステートの生成
	HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(
		&graphicsPipelineStateDesc, 
		IID_PPV_ARGS(&graphicsPipelineState_));
	assert(SUCCEEDED(hr));
}

bool ParticleManager::IsCollision(const AABB& aabb, const Vector3& point)
{
	return (aabb.min.x <= point.x && aabb.max.x >= point.x) &&
	       (aabb.min.y <= point.y && aabb.max.y >= point.y) &&
	       (aabb.min.z <= point.z && aabb.max.z >= point.z);
}

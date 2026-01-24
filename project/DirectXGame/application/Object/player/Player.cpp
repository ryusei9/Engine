#include "Player.h"
#include <Object3dCommon.h>
#include <CollisionTypeIdDef.h>
#include <PlayerBullet.h>
#include <PlayerChargeBullet.h>
#include "JsonLoader.h"
#include <cmath>

#ifdef USE_IMGUI
#include <imgui.h>
#endif

Player::Player()
{
	// シリアルナンバーを振る
	serialNumber_ = sNextSerialNumber_;
	// 次のシリアルナンバーに1を足す
	++sNextSerialNumber_;
}

void Player::Initialize()
{
	// デフォルトパラメータで初期化
	Initialize("playerParameters");
}

void Player::Initialize(const std::string& parameterFileName)
{
	// パラメータファイルから読み込み
	parameters_ = JsonLoader::LoadPlayerParameters(parameterFileName);

	// パラメータを各メンバ変数に適用
	moveSpeed_ = parameters_.moveSpeed;
	radius_ = parameters_.radius;
	respawnTimer_ = parameters_.respawnWaitSec;

	// プレイヤーの初期化
	BaseCharacter::Initialize();

	// プレイヤーのコライダーの設定
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kPlayer));

	worldTransform_.Initialize();
	// 初期Transform
	worldTransform_.SetScale(parameters_.initScale);
	worldTransform_.SetRotate(parameters_.initRotate);
	worldTransform_.SetTranslate(parameters_.initTranslate);

	// プレイヤーのカメラを取得
	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();

	// 3Dオブジェクト
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(parameters_.modelFileName);

	SetRadius(parameters_.radius); // コライダーの半径を設定

	// パーティクルグループの作成
	particleManager_ = ParticleManager::GetInstance();
	particleManager_->CreateParticleGroup("thruster", parameters_.thrusterTexture);
	particleManager_->CreateParticleGroup("explosion", parameters_.explosionTexture);

	// エミッター初期化
	thrusterEmitter_ = std::make_unique<ParticleEmitter>(ParticleManager::GetInstance(), "thruster");
	thrusterEmitter_->SetParticleRate(parameters_.thrusterRate);
	thrusterEmitter_->SetParticleCount(parameters_.thrusterCount);
	thrusterEmitter_->SetThruster(true); // スラスターエミッターを有効化

	explosionEmitter_ = std::make_unique<ParticleEmitter>(ParticleManager::GetInstance(), "explosion");
	explosionEmitter_->SetUseRingParticle(true);
	explosionEmitter_->SetExplosion(true);
}

void Player::Update()
{
	// プレイヤーの更新
	BaseCharacter::Update();

	// 弾の削除
	bullets_.remove_if([](std::unique_ptr<PlayerBullet>& bullet) {
		if (!bullet->IsAlive()) {
			bullet.reset();
			return true;
		}
		return false;
	});

	// 弾の更新
	for (auto& bullet : bullets_) {
		bullet->Update();
	}

	// プレイヤーの移動・攻撃
	if (controlEnabled_) {
		Move();
		Attack();
	}

	// スラスター方向
	float yRad = worldTransform_.GetRotate().y;
	Vector3 leftDir = { -std::cos(yRad), 0.0f, std::sin(yRad) };

	// 勢いの強さ（パラメータから取得）
	float power = parameters_.thrusterBasePower + velocity_.x * parameters_.thrusterVelocityMultiplier;
	Vector3 particleVelocity = leftDir * power;

	// パーティクル更新
	const Vector3& pos = worldTransform_.GetTranslate();
	thrusterEmitter_->SetPosition(pos - Vector3(parameters_.thrusterOffsetX, 0.0f, 0.0f));
	thrusterEmitter_->SetVelocity(particleVelocity);
	thrusterEmitter_->Update();

	// ワールド変換の更新
	worldTransform_.Update();

	// 3Dオブジェクトへ反映
	object3d_->SetCamera(camera_);
	object3d_->SetTranslate(worldTransform_.GetTranslate());
	object3d_->SetRotate(worldTransform_.GetRotate());
	object3d_->SetScale(worldTransform_.GetScale());
	object3d_->Update();
}

void Player::Draw()
{
	// 死亡していなければ描画
	if (!isAlive_) {
		return;
	}
	BaseCharacter::Draw();
	for (auto& bullet : bullets_) {
		bullet->Draw();
	}
}

void Player::Move()
{
	// 速度をリセット
	velocity_ = { 0.0f, 0.0f, 0.0f };

	// 入力
	if (input_->PushKey(DIK_W)) { 
		velocity_.y += moveSpeed_; 
	}
	if (input_->PushKey(DIK_S)) { 
		velocity_.y -= moveSpeed_; 
	}
	if (input_->PushKey(DIK_A)) { 
		velocity_.x -= moveSpeed_; 
	}
	if (input_->PushKey(DIK_D)) { 
		velocity_.x += moveSpeed_; 
	}

	// 速度を座標に反映
	worldTransform_.SetTranslate(worldTransform_.GetTranslate() + velocity_);
}

void Player::Attack()
{
	// チャージ
	if (input_->PushKey(DIK_SPACE)) {
		if (!isCharging_) {
			isCharging_ = true;
			chargeTime_ = 0.0f;
		}
		chargeTime_ += PlayerDefaults::kDelta60Hz; // 1フレーム分加算
		if (chargeTime_ >= parameters_.chargeReadySec) {
			chargeReady_ = true;
		}
	}
	else {
		// チャージショット
		if (isCharging_ && chargeReady_) {
			auto chargeBullet = std::make_unique<PlayerChargeBullet>();
			chargeBullet->Initialize(worldTransform_.GetTranslate(), "playerChargeBulletParameters");
			chargeBullet->Update();
			bullets_.push_back(std::move(chargeBullet));
		}
		// 通常ショット
		else if (isCharging_ && chargeTime_ < parameters_.chargeReadySec) {
			auto bullet = std::make_unique<PlayerBullet>();
			bullet->Initialize(worldTransform_.GetTranslate(), "playerBulletParameters");
			bullet->Update();
			bullets_.push_back(std::move(bullet));
		}
		// リセット
		isCharging_ = false;
		chargeTime_ = 0.0f;
		chargeReady_ = false;
	}
}

void Player::OnCollision(Collider* other)
{
	if (isAlive_) {
		if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeIdDef::kEnemy) ||
			other->GetTypeID() == static_cast<uint32_t>(CollisionTypeIdDef::kEnemyBullet)) {
			isAlive_ = false;
			respawnTimer_ = parameters_.respawnWaitSec;
			PlayDeathParticleOnce();
		}
	}
}

void Player::DrawImGui()
{
#ifdef USE_IMGUI
	object3d_->DrawImGui();
	ImGui::Begin("Player Info");
	ImGui::Text("Charge Time: %.2f seconds", chargeTime_);
	ImGui::Text("Is Charging: %s", isCharging_ ? "Yes" : "No");
	ImGui::Text("Charge Ready: %s", chargeReady_ ? "Yes" : "No");
	ImGui::End();
#endif
}

Vector3 Player::GetCenterPosition() const
{
	const Vector3 offset = { 0.0f, 0.0f, 0.0f };
	return worldTransform_.GetTranslate() + offset;
}

void Player::PlayDeathParticleOnce()
{
	if (!hasPlayedDeathParticle_) {
		if (explosionEmitter_) {
			explosionEmitter_->SetPosition(worldTransform_.GetTranslate());
			explosionEmitter_->SetParticleRate(static_cast<uint32_t>(parameters_.explosionRate));
			explosionEmitter_->SetParticleCount(parameters_.explosionCount);
			explosionEmitter_->Update();
		}
		hasPlayedDeathParticle_ = true;
	}
}

void Player::SetParameters(const PlayerParameters& parameters)
{
	parameters_ = parameters;
	// パラメータを各メンバ変数に適用
	moveSpeed_ = parameters_.moveSpeed;
	radius_ = parameters_.radius;
	SetRadius(parameters_.radius);
}
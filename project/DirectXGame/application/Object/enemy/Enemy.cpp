#include "Enemy.h"
#include <CollisionTypeIdDef.h>
#include <Object3dCommon.h>
#include <ParticleEmitter.h>
#include <PlayerChargeBullet.h>
#include "JsonLoader.h"
#include <imgui.h>
#include <CurveLibrary.h>

namespace {
	constexpr float kUpdateDeltaTime = 1.0f / 60.0f;
	constexpr float kDeathDeltaTime = 1.0f / 120.0f;
	constexpr float kRotationSpeedHalf = 0.005f;
}

Enemy::Enemy()
{
	// シリアルナンバーを振る
	serialNumber_ = sNextSerialNumber_;
	// 次のシリアルナンバーに1を足す
	++sNextSerialNumber_;
}

void Enemy::Initialize()
{
	// デフォルトパラメータで初期化
	Initialize("enemyParameters");
}

void Enemy::Initialize(const std::string& parameterFileName)
{
	// パラメータファイルから読み込み
	parameters_ = JsonLoader::LoadEnemyParameters(parameterFileName);

	// パラメータを各メンバ変数に適用
	moveSpeed_ = parameters_.moveSpeed;
	hp_ = parameters_.initialHp;
	respawnTime_ = parameters_.respawnTimeSec;
	radius_ = parameters_.colliderRadius;
	shotInterval_ = parameters_.shotIntervalSec;
	deathTimer_ = parameters_.deathDurationSec;
	fallSpeed_ = parameters_.fallSpeed;
	rotationSpeed_ = parameters_.rotationSpeed;

	// 敵の初期化
	BaseCharacter::Initialize();
	// 敵のコライダーの設定
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kEnemy));

	// 敵のワールド変換を初期化
	worldTransform_.Initialize();
	// 初期Transform（定数で意味付け）
	worldTransform_.SetScale(Vector3{ 1.0f, 1.0f, 1.0f });
	worldTransform_.SetRotate(Vector3{ 0.0f, 0.0f, 0.0f });
	worldTransform_.SetTranslate(Vector3{ 10.0f, 0.0f, 0.0f });

	// 敵のカメラを取得
	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();

	// 敵の3Dオブジェクトを生成・初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize("enemy.obj");

	// パーティクルマネージャの初期化
	particleManager_ = ParticleManager::GetInstance();

	// 敵死亡時のパーティクル設定
	particleManager_->GetInstance()->SetParticleType(ParticleType::Explosion);
	// テクスチャ"circle2"を使用
	particleManager_->GetInstance()->CreateParticleGroup("explosion", "resources/circle2.png");
	particleManager_->GetInstance()->CreateParticleGroup("smoke", "resources/circle2.png");

	// 敵死亡時のパーティクルエミッターを初期化
	enemyDeathEmitter_ = std::make_unique<ParticleEmitter>(particleManager_, "explosion");
	enemyDeathEmitter_->SetUseRingParticle(true);
	enemyDeathEmitter_->SetExplosion(true);

	// 敵ヒット時のパーティクル設定
	enemyHitEmitter_ = std::make_unique<ParticleEmitter>(particleManager_, "explosion");
	enemyHitEmitter_->SetUseRingParticle(true);
	enemyHitEmitter_->SetExplosion(true);

	// 煙用のパーティクルエミッターを初期化
	particleManager_->GetInstance()->SetParticleType(ParticleType::Normal);
	smokeEmitter_ = std::make_unique<ParticleEmitter>(particleManager_, "smoke");
	smokeEmitter_->SetParticleRate(parameters_.smokeParticleRate);
	smokeEmitter_->SetParticleCount(parameters_.smokeParticleCount);
	smokeEmitter_->SetSmoke(true);

	SetRadius(parameters_.colliderRadius);

	// 敵の攻撃パターンを初期化
	attack_ = std::make_unique<EnemyAttack>();
	attack_->Initialize("enemyAttackParameters"); // パラメータファイルから読み込み
	// デフォルトの攻撃パターンを設定
	attack_->SetPattern(1);
}

void Enemy::Update()
{
	// 弾の削除
	bullets_.remove_if([](std::unique_ptr<EnemyBullet>& bullet) {
		return !bullet->IsAlive();
	});

	// 弾の更新
	for (auto& bullet : bullets_) {
		bullet->Update();
	}

	if (!curveMoveManager_ && moveCurve_) {
		StartCurveMove(*moveCurve_);
	}
	
	// 生きてる時だけ通常ロジック
	if (state_ == EnemyState::Alive) {
		BaseCharacter::Update();
		bool isCurveMoving = false;

		if (curveMoveManager_) {
			curveMoveManager_->Update(kUpdateDeltaTime);
			SetPosition(curveMoveManager_->GetPosition());

			isCurveMoving = !curveMoveManager_->IsFinished();

			if (!isCurveMoving) {
				curveMoveManager_.reset();
			}
		}

		// ↓ カーブ中は「移動ロジック」だけをスキップ
		if (!isCurveMoving) {
			// 通常移動・AI・攻撃
			if (controlEnabled_) {
				attack_->Update(this, player_, bullets_, kUpdateDeltaTime);
			}
		}
		
		if (hp_ <= 0) {
			// 死亡演出へ移行
			state_ = EnemyState::Dying;
			object3d_->SetPointLight(0.0f);
			object3d_->SetDirectionalLight(1.0f);
			object3d_->SetMaterialColor(Vector4(0.3f, 0.3f, 0.3f, 1.0f));
		}
	}

	// ワールド変換の更新
	worldTransform_.Update();

	// 敵のワールド変換を更新
	object3d_->SetCamera(camera_);
	object3d_->SetScale(worldTransform_.GetScale());
	object3d_->SetRotate(worldTransform_.GetRotate());
	object3d_->SetTranslate(worldTransform_.GetTranslate());
	object3d_->Update();

	// 死亡演出
	if (state_ == EnemyState::Dying) {
		deathTimer_ -= kDeathDeltaTime;

		// 回転
		Vector3 r = worldTransform_.GetRotate();
		r.x += kRotationSpeedHalf;
		r.y -= parameters_.rotationSpeed;
		r.z -= parameters_.rotationSpeed;
		worldTransform_.SetRotate(r);

		// 落下（-Y方向）
		Vector3 t = worldTransform_.GetTranslate();
		t.y += parameters_.fallSpeed;
		worldTransform_.SetTranslate(t);

		// 煙の向き・速度
		Vector3 leftDir = { 0.0f, parameters_.smokeUpwardForce, 0.0f };
		float power = parameters_.smokePower + parameters_.smokePowerMultiplier * 1.5f;
		Vector3 particleVelocity = leftDir * power;

		// パーティクル更新
		smokeEmitter_->SetPosition(worldTransform_.GetTranslate());
		smokeEmitter_->SetVelocity(particleVelocity);
		smokeEmitter_->Update();

		if (deathTimer_ <= 0.0f) {
			state_ = EnemyState::Dead;
		}
	}

	// ここでパーティクル発動＆削除
	if (state_ == EnemyState::Dead && !deathEffectPlayed_) {
		PlayDeathParticleOnce();
		deathEffectPlayed_ = true;
#ifdef _DEBUG
		// 敵が死んでいる場合
		// 敵のリスポーンタイムを減少
		respawnTime_ -= kUpdateDeltaTime;
		if (respawnTime_ <= 0.0f) {
			// リスポーンタイムが0以下になったら
			state_ = EnemyState::Alive;
			hp_ = parameters_.initialHp;
			respawnTime_ = parameters_.respawnTimeSec;
			hasPlayedDeathParticle_ = false;
			deathEffectPlayed_ = false;
		}
#endif
	}
}

void Enemy::Draw()
{
	// 敵が生きている場合のみ描画
	if (state_ == EnemyState::Alive || state_ == EnemyState::Dying) {
		// 敵の描画
		BaseCharacter::Draw();
		// 弾の描画
		for (auto& bullet : bullets_) {
			bullet->Draw();
		}
	}
}

void Enemy::DrawImGui()
{
#ifdef USE_IMGUI
	ImGui::Begin("enemy");
	// ImGuiで敵の情報を表示
	ImGui::Text("Enemy Serial Number: %u", serialNumber_);
	ImGui::Text("HP: %d", hp_);
	const Vector3& pos = worldTransform_.GetTranslate();
	ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
	ImGui::Text("Alive: %s", isAlive_ ? "Yes" : "No");
	ImGui::Text("Respawn Time: %.2f seconds", respawnTime_);
	// 攻撃パターンの選択
	attack_->DrawImGui();
	object3d_->DrawImGui();
	ImGui::End();
#endif
}

void Enemy::Move()
{
}

void Enemy::Attack()
{
}

void Enemy::OnCollision(Collider* other)
{
	// 敵の衝突判定
	// 生きていない場合は無視
	if (state_ != EnemyState::Alive) {
		return;
	}

	// チャージ弾と衝突した場合
	if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeIdDef::kPlayerChargeBullet)) {
		auto* chargeBullet = dynamic_cast<PlayerChargeBullet*>(other);
		if (chargeBullet) {
			hp_ -= static_cast<int>(chargeBullet->GetDamage());
		}
		PlayHitParticle();
	}
	else if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeIdDef::kPlayerBullet)) {
		// プレイヤー弾と衝突した場合
		hp_ -= 1;
		PlayHitParticle();
	}
}

void Enemy::PlayDeathParticleOnce()
{
	// 敵死亡時に一度だけパーティクルを出す
	if (!hasPlayedDeathParticle_) {
		if (enemyDeathEmitter_) {
			enemyDeathEmitter_->SetPosition(worldTransform_.GetTranslate());
			enemyDeathEmitter_->SetParticleRate(parameters_.deathParticleRate);
			enemyDeathEmitter_->SetParticleCount(parameters_.deathParticleCount);
			enemyDeathEmitter_->Update();
		}
		hasPlayedDeathParticle_ = true;
	}
}

void Enemy::PlayHitParticle()
{
	if (!hasPlayedHitParticle_) {
		if (enemyHitEmitter_) {
			enemyHitEmitter_->SetPosition(worldTransform_.GetTranslate());
			enemyHitEmitter_->SetParticleRate(parameters_.hitParticleRate);
			enemyHitEmitter_->SetParticleCount(parameters_.hitParticleCount);
			enemyHitEmitter_->Update();
		}
		hasPlayedHitParticle_ = true;
	}
}

void Enemy::StartCurveMove(const CurveData& curve)
{
	curveMoveManager_ = std::make_unique<CurveMoveManager>();
	curveMoveManager_->Start(CurveData(curve)); // ← 明示コピー
}

Vector3 Enemy::GetCenterPosition() const
{
	const Vector3 offset = { 0.0f, 0.0f, 0.0f };
	return worldTransform_.GetTranslate() + offset;
}

void Enemy::SetParameters(const EnemyParameters& parameters)
{
	parameters_ = parameters;
	// パラメータを各メンバ変数に適用
	moveSpeed_ = parameters_.moveSpeed;
	radius_ = parameters_.colliderRadius;
	shotInterval_ = parameters_.shotIntervalSec;
	fallSpeed_ = parameters_.fallSpeed;
	rotationSpeed_ = parameters_.rotationSpeed;
}


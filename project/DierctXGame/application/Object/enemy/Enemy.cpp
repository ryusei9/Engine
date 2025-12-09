#include "Enemy.h"
#include <CollisionTypeIdDef.h>
#include <Object3dCommon.h>
#include <ParticleEmitter.h>
#include <PlayerChargeBullet.h>
Enemy::Enemy()
{
	// シリアルナンバーを振る
	serialNumber_ = nextSerialNumber_;
	// 次のシリアルナンバーに1を足す
	++nextSerialNumber_;
}

void Enemy::Initialize()
{
	// 敵の初期化
	BaseCharacter::Initialize();
	// 敵のコライダーの設定
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kEnemy));

	// 敵のワールド変換を初期化
	worldTransform_.Initialize();
	// 敵のワールド変換を初期化
	worldTransform_.scale_ = { 1.0f,1.0f,1.0f };
	worldTransform_.rotate_ = { 0.0f,0.0f,0.0f };
	worldTransform_.translate_ = { 10.0f,0.0f,0.0f };
	// 敵のカメラを取得
	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();

	// 敵の3Dオブジェクトを生成
	object3d_ = std::make_unique<Object3d>();
	// 敵の3Dオブジェクトを初期化
	object3d_->Initialize("enemy.obj");

	// パーティクルマネージャの初期化
	particleManager = ParticleManager::GetInstance();

	// 敵死亡時のパーティクル設定
	particleManager->GetInstance()->SetParticleType(ParticleType::Explosion);
	// テクスチャ"circle2"を使用
	particleManager->GetInstance()->CreateParticleGroup("explosion", "resources/circle2.png");
	particleManager->GetInstance()->CreateParticleGroup("smoke","resources/circle2.png");

	// 敵死亡時のパーティクルエミッターを初期化
	enemyDeathEmitter_ = std::make_unique<ParticleEmitter>(particleManager, "explosion");
	enemyDeathEmitter_->SetUseRingParticle(true);
	enemyDeathEmitter_->SetExplosion(true); // 爆発エミッターに設定

	// 敵ヒット時のパーティクル設定
	enemyHitEmitter_ = std::make_unique<ParticleEmitter>(particleManager, "explosion");
	enemyHitEmitter_->SetUseRingParticle(true);
	enemyHitEmitter_->SetExplosion(true);

	// 煙用のパーティクルエミッターを初期化
	particleManager->GetInstance()->SetParticleType(ParticleType::Normal);
	smokeEmitter_ = std::make_unique<ParticleEmitter>(particleManager, "smoke");
	smokeEmitter_->SetParticleRate(60);
	smokeEmitter_->SetParticleCount(3);
	smokeEmitter_->SetSmoke(true); 

	SetRadius(0.4f); // コライダーの半径を設定

	// 敵の攻撃パターンを初期化
	attack_ = std::make_unique<EnemyAttack>();
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
	// 生きてる時だけ通常ロジック
	if (state_ == EnemyState::Alive) {
		BaseCharacter::Update();
		Move();
		// 敵の攻撃
		if (controlEnabled_) {
			attack_->Update(this, player_, bullets_, 1.0f / 60.0f);
		}
		if (hp_ <= 0) {
			// 死亡演出へ移行
			state_ = EnemyState::Dying;
			object3d_->SetPointLight(0.0f); // 光消す(optional)
			object3d_->SetDirectionalLight(1.0f);  // 光消す(optional)
			//object3d_->SetSpotLight(2.0f);  // 光消す(optional)
			object3d_->SetMaterialColor(Vector4(0.3f, 0.3f, 0.3f, 1.0f)); // グレー化
		}
	}
	
	// ワールド変換の更新
	worldTransform_.Update();
	// 敵のワールド変換を更新
	object3d_->SetCamera(camera_);
	object3d_->SetScale(worldTransform_.scale_);
	object3d_->SetRotate(worldTransform_.rotate_);
	object3d_->SetTranslate(worldTransform_.translate_);
	object3d_->Update();
	
	
	
	// 死亡演出
	if (state_ == EnemyState::Dying) {
		deathTimer_ -= 1.0f / 120.0f;

		// 回転
		worldTransform_.rotate_.x += rotationSpeed_ / 2.0f;
		worldTransform_.rotate_.y -= rotationSpeed_;
		worldTransform_.rotate_.z -= rotationSpeed_;

		// 落下（-Y方向）
		worldTransform_.translate_.y += fallSpeed_;

		float yRad = worldTransform_.rotate_.y;
		Vector3 leftDir = {
			0.0f, // X
			2.0f,        // Y
			0.0f   // Z
		};

		// 勢いの強さはvelocityの大きさで調整
		float power = 2.0f + 0.1f * 1.5f; // 例: X速度で強さを変える
		Vector3 particleVelocity = leftDir * power;

		// パーティクルの発生位置（プレイヤーの中心 or 左側に少しオフセットしてもOK）
		Vector3 emitPos = worldTransform_.translate_;
		// Update
		smokeEmitter_->SetPosition(worldTransform_.translate_);
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
		respawnTime_ -= 1.0f / 60.0f;
		if (respawnTime_ <= 0.0f)
		{
			// リスポーンタイムが0以下になったら
			state_ = EnemyState::Alive;
			hp_ = 1;
			respawnTime_ = 3.0f; // リスポーンタイムを3秒に設定
			hasPlayedDeathParticle_ = false;
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
		for (auto& bullet : bullets_) bullet->Draw();
	}
}

void Enemy::DrawImGui()
{
#ifdef USE_IMGUI
	ImGui::Begin("enemy");
	// ImGuiで敵の情報を表示
	ImGui::Text("Enemy Serial Number: %u", serialNumber_);
	ImGui::Text("HP: %d", hp_);
	ImGui::Text("Position: (%.2f, %.2f, %.2f)", worldTransform_.translate_.x, worldTransform_.translate_.y, worldTransform_.translate_.z);
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
	if (state_ != EnemyState::Alive) return;
	// チャージ弾と衝突した場合
	if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeIdDef::kPlayerChargeBullet))
	{
		auto* chargeBullet = dynamic_cast<PlayerChargeBullet*>(other);
		if (chargeBullet) {
			hp_ -= static_cast<int>(chargeBullet->GetDamage()); // チャージ弾
		}
		//PlayDeathParticleOnce(); // ここで一度だけパーティクルを出す
		PlayHitParticle();
	} else if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeIdDef::kPlayerBullet))
	{
		// プレイヤー弾と衝突した場合
		hp_ -= 1;
		PlayHitParticle();
		//PlayDeathParticleOnce(); // ここで一度だけパーティクルを出す
	}
	
}

void Enemy::PlayDeathParticleOnce()
{
	// 敵死亡時に一度だけパーティクルを出す
	if (!hasPlayedDeathParticle_) {
		if (enemyDeathEmitter_) {
			enemyDeathEmitter_->SetPosition(worldTransform_.translate_); // 位置をセット
			enemyDeathEmitter_->SetParticleRate(8); // 必要に応じて発生数を調整
			enemyDeathEmitter_->SetParticleCount(8);
			// ここでパーティクルを即時発生させるメソッドがあれば呼ぶ
			enemyDeathEmitter_->Update();
		}
		hasPlayedDeathParticle_ = true;
	}
}

void Enemy::PlayHitParticle()
{
	if (!hasPlayedHitParticle_) {
		if (enemyHitEmitter_) {
			enemyHitEmitter_->SetPosition(worldTransform_.translate_); // 位置をセット
			enemyHitEmitter_->SetParticleRate(1); // 必要に応じて発生数を調整
			enemyHitEmitter_->SetParticleCount(0);
			// ここでパーティクルを即時発生させるメソッドがあれば呼ぶ
			enemyHitEmitter_->Update();
		}
		hasPlayedHitParticle_ = true;
	}
}

Vector3 Enemy::GetCenterPosition() const
{
	const Vector3 offset = { 0.0f, 0.0f, 0.0f }; // エネミーの中心を考慮
	Vector3 worldPosition = worldTransform_.translate_ + offset;
	return worldPosition;
}



#include "Enemy.h"
#include <CollisionTypeIdDef.h>
#include <Object3dCommon.h>
#include <ParticleEmitter.h>
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

	worldTransform_.Initialize();
	// 敵のワールド変換を初期化
	worldTransform_.scale_ = { 0.5f,0.5f,0.5f };
	worldTransform_.rotate_ = { 0.0f,0.0f,0.0f };
	worldTransform_.translate_ = { 3.0f,0.0f,0.0f };
	// 敵のカメラを取得
	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();
	object3d_ = std::make_unique<Object3d>();
	// 敵の3Dオブジェクトを初期化
	object3d_->Initialize("Boss.obj");

	// パーティクルマネージャの初期化
	particleManager = ParticleManager::GetInstance();
	particleManager->GetInstance()->SetParticleType(ParticleType::Explosion);
	// テクスチャ"circle2"を使用
	particleManager->GetInstance()->CreateParticleGroup("explosion", "resources/circle2.png");

	// 敵死亡時のパーティクルエミッターを初期化
	enemyDeathEmitter_ = std::make_unique<ParticleEmitter>(particleManager, "explosion");
	enemyDeathEmitter_->SetUseRingParticle(true); // 必要に
	enemyDeathEmitter_->SetExplosion(true); // 爆発エミッターに設定
	SetRadius(0.4f); // コライダーの半径を設定

	attack_ = std::make_unique<EnemyAttack>();
}

void Enemy::Update()
{
	// 弾の削除
	bullets_.remove_if([](std::unique_ptr<EnemyBullet>& bullet) {
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
	// 敵の更新
	BaseCharacter::Update();
	worldTransform_.Update();
	// 敵のワールド変換を更新
	object3d_->SetCamera(camera_);
	object3d_->SetScale(worldTransform_.scale_);
	object3d_->SetRotate(worldTransform_.rotate_);
	object3d_->SetTranslate(worldTransform_.translate_);
	object3d_->Update();
	// 敵の移動
	Move();
	// 敵の攻撃
	//Attack();
	attack_->Update(this, player_, bullets_, 1.0f / 60.0f);
	if (hp_ <= 0)
	{
		// 敵のHPが0以下になったら
		isAlive_ = false;
	}
	if (!isAlive_) {
		// 敵が死んでいる場合
		// 敵のリスポーンタイムを減少
		respawnTime_ -= 1.0f / 60.0f;
		if (respawnTime_ <= 0.0f)
		{
			// リスポーンタイムが0以下になったら
			isAlive_ = true;
			hp_ = 1;
			respawnTime_ = 3.0f; // リスポーンタイムを3秒に設定
			hasPlayedDeathParticle_ = false;
		}
	}
}

void Enemy::Draw()
{
	if (isAlive_) {
		// 敵の描画
		BaseCharacter::Draw();
		for (auto& bullet : bullets_) {
			bullet->Draw();
		}
	}
}

void Enemy::DrawImGui()
{
	ImGui::Begin("enemy");
	// ImGuiで敵の情報を表示
	ImGui::Text("Enemy Serial Number: %u", serialNumber_);
	ImGui::Text("HP: %d", hp_);
	ImGui::Text("Position: (%.2f, %.2f, %.2f)", worldTransform_.translate_.x, worldTransform_.translate_.y, worldTransform_.translate_.z);
	ImGui::Text("Alive: %s", isAlive_ ? "Yes" : "No");
	ImGui::Text("Respawn Time: %.2f seconds", respawnTime_);
	// 攻撃パターンの選択
	attack_->DrawImGui();
	ImGui::End();
}

void Enemy::Move()
{
}

void Enemy::Attack()
{
	if (!isAlive_) return;
	shotTimer_ -= 1.0f / 60.0f;
	if (shotTimer_ <= 0.0f) {
		// プレイヤーの方向に発射（例：X軸負方向に発射）
		Vector3 velocity = { -0.1f, 0.0f, 0.0f };
		auto bullet = std::make_unique<EnemyBullet>();
		bullet->Initialize(worldTransform_.translate_, velocity);
		bullet->Update();
		bullets_.push_back(std::move(bullet));
		shotTimer_ = shotInterval_;
	}
}

void Enemy::OnCollision(Collider* other)
{
	// 敵の衝突判定
	if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeIdDef::kPlayerBullet))
	{
		// プレイヤー弾と衝突した場合
		hp_ -= 1;
		PlayDeathParticleOnce(); // ここで一度だけパーティクルを出す
	}
}

void Enemy::PlayDeathParticleOnce()
{
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

Vector3 Enemy::GetCenterPosition() const
{
	const Vector3 offset = { 0.0f, 0.0f, 0.0f }; // エネミーの中心を考慮
	Vector3 worldPosition = worldTransform_.translate_ + offset;
	return worldPosition;
}



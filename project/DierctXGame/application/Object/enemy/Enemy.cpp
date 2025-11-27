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

	// 敵死亡時のパーティクルエミッターを初期化
	enemyDeathEmitter_ = std::make_unique<ParticleEmitter>(particleManager, "explosion");
	enemyDeathEmitter_->SetUseRingParticle(true);
	enemyDeathEmitter_->SetExplosion(true); // 爆発エミッターに設定
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
	// 敵の更新
	BaseCharacter::Update();
	// ワールド変換の更新
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
	if (controlEnabled_) {
		attack_->Update(this, player_, bullets_, 1.0f / 60.0f);
	}
	// 敵の死亡条件
	if (hp_ <= 0)
	{
		// 敵のHPが0以下になったら
		isAlive_ = false;
	}
	if (!isAlive_) {
#ifdef _DEBUG
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
#endif
	}
}

void Enemy::Draw()
{
	// 敵が生きている場合のみ描画
	if (isAlive_ || controlEnabled_) {
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
	// チャージ弾と衝突した場合
	if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeIdDef::kPlayerChargeBullet))
	{
		auto* chargeBullet = dynamic_cast<PlayerChargeBullet*>(other);
		if (chargeBullet) {
			hp_ -= static_cast<int>(chargeBullet->GetDamage()); // チャージ弾
		}
		PlayDeathParticleOnce(); // ここで一度だけパーティクルを出す
	} else if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeIdDef::kPlayerBullet))
	{
		// プレイヤー弾と衝突した場合
		hp_ -= 1;
		PlayDeathParticleOnce(); // ここで一度だけパーティクルを出す
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

Vector3 Enemy::GetCenterPosition() const
{
	const Vector3 offset = { 0.0f, 0.0f, 0.0f }; // エネミーの中心を考慮
	Vector3 worldPosition = worldTransform_.translate_ + offset;
	return worldPosition;
}



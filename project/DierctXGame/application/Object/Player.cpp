#include "Player.h"
#include <Object3dCommon.h>
#include <CollisionTypeIdDef.h>
#include <PlayerBullet.h>
Player::Player()
{
	// シリアルナンバーを振る
	serialNumber_ = nextSerialNumber_;
	// 次のシリアルナンバーに1を足す
	++nextSerialNumber_;
}
void Player::Initialize()
{
	// プレイヤーの初期化
	BaseCharacter::Initialize();

	// プレイヤーのコライダーの設定
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kPlayer));

	worldTransform_.Initialize();
	// プレイヤーのワールド変換を初期化
	worldTransform_.scale_ = { 1.0f,1.0f,1.0f };
	worldTransform_.rotate_ = { 0.0f,0.0f,0.0f };
	worldTransform_.translate_ = { 0.0f,1.0f,0.0f };

	// プレイヤーのカメラを取得
	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();

	object3d_ = std::make_unique<Object3d>();

	// プレイヤーの3Dオブジェクトを初期化
	object3d_->Initialize("player.obj");

	SetRadius(radius_); // コライダーの半径を設定

	particleManager_->GetInstance()->CreateParticleGroup("thruster", "resources/circle2.png");
	particleManager_->GetInstance()->CreateParticleGroup("explosion", "resources/circle2.png");

	// 初期化
	thrusterEmitter_ = std::make_unique<ParticleEmitter>(ParticleManager::GetInstance(), "thruster");
	thrusterEmitter_->SetParticleRate(60); // 1秒間に60個
	thrusterEmitter_->SetParticleCount(3);
	thrusterEmitter_->SetThruster(true); // スラスターエミッターを有効化

	explosionEmitter_ = std::make_unique<ParticleEmitter>(ParticleManager::GetInstance(), "explosion");
	explosionEmitter_->SetUseRingParticle(true); // リングパーティクルを使用
	explosionEmitter_->SetExplosion(true); // 爆発エミッターを有効化

}

void Player::Update()
{// プレイヤーの更新
	if (!isAlive_) {
		respawnTimer_ -= 1.0f / 60.0f;
		if (respawnTimer_ <= 0.0f) {
			// 復活
			isAlive_ = true;
			worldTransform_.translate_ = { 0.0f, 0.0f, 0.0f }; // 初期位置に戻す
			hp_ = 1;
			hasPlayedDeathParticle_ = false;
		}
		return; // 死亡中は何もしない
	}
	BaseCharacter::Update();

	// 弾の削除
	bullets_.remove_if([](std::unique_ptr<PlayerBullet>& bullet) {
		// 弾が死んでいる場合
		if (!bullet->IsAlive()) {

			// リセット
			bullet.reset();
			return true;
		}

		return false;
		});

	// 弾の更新
	for (auto& bullet : bullets_) {
		bullet->Update();
	}

	// プレイヤーの移動
	Move();
	Attack();
	worldTransform_.Update();

	
	// プレイヤーのワールド変換を更新
	object3d_->SetCamera(camera_);
	object3d_->SetTranslate(worldTransform_.translate_);
	object3d_->SetRotate(worldTransform_.rotate_);
	object3d_->SetScale(worldTransform_.scale_);
	object3d_->Update();
}

void Player::Draw()
{

	if (!isAlive_) return;
	BaseCharacter::Draw();
	for (auto& bullet : bullets_) {
		bullet->Draw();
	}

}

void Player::Move()
{

	float yRad = worldTransform_.rotate_.y;
	Vector3 leftDir = {
		-cosf(yRad), // X
		0.0f,        // Y
		sinf(yRad)   // Z
	};
	// 速度をリセット
	velocity_ = { 0.0f, 0.0f, 0.0f };

	// 入力に応じて速度を設定
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
	worldTransform_.translate_ += velocity_;

	// 勢いの強さはvelocityの大きさで調整
	float power = 2.0f + velocity_.x * 1.5f; // 例: X速度で強さを変える
	Vector3 particleVelocity = leftDir * power;

	// パーティクルの発生位置（プレイヤーの中心 or 左側に少しオフセットしてもOK）
	Vector3 emitPos = worldTransform_.translate_;
	// Update
	thrusterEmitter_->SetPosition(worldTransform_.translate_ - Vector3(0.2f,0.0f,0.0f));
	thrusterEmitter_->SetVelocity(particleVelocity);
	thrusterEmitter_->Update();
}

void Player::Attack()
{// プレイヤーの攻撃
	  // プレイヤーの攻撃
	if (input_->TriggerKey(DIK_SPACE)) {
		isShot_ = true;
	}
	if (isShot_) {
		// 弾を生成
		auto bullet = std::make_unique<PlayerBullet>();

		// 弾の初期化
		bullet->Initialize(worldTransform_.translate_);

		// 弾の位置をプレイヤーの位置に設定
		//bullet->SetTranslate(worldTransform_.translate_);
		bullet->Update(); // ←ここで1回Update
		// 弾をリストに追加
		bullets_.push_back(std::move(bullet));
		isShot_ = false;
	}
}

void Player::OnCollision(Collider* other)
{// プレイヤーの衝突判定
	if (isAlive_) {
		if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeIdDef::kEnemy)) {
			// 敵と衝突した場合
			isAlive_ = false;
			respawnTimer_ = 2.0f; // 2秒後に復活
			PlayDeathParticleOnce();
		}
		if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeIdDef::kEnemyBullet)) {
			isAlive_ = false;
			respawnTimer_ = 2.0f; // 2秒後に復活
			PlayDeathParticleOnce();
		}
	}
}

Vector3 Player::GetCenterPosition() const
{
	const Vector3 offset = { 0.0f, 0.0f, 0.0f }; // プレイヤーの中心を考慮
	Vector3 worldPosition = worldTransform_.translate_ + offset;
	return worldPosition;
}

void Player::PlayDeathParticleOnce() {
	if (!hasPlayedDeathParticle_) {
		if (explosionEmitter_) {
			explosionEmitter_->SetPosition(worldTransform_.translate_); // 位置をセット
			explosionEmitter_->SetParticleRate(1); // 必要に応じて発生数を調整
			explosionEmitter_->SetParticleCount(0);
			// ここでパーティクルを即時発生させるメソッドがあれば呼ぶ
			explosionEmitter_->Update();
		}
		hasPlayedDeathParticle_ = true;
	}
}
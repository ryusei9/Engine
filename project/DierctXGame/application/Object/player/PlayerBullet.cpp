#include "PlayerBullet.h"
#include <CollisionTypeIdDef.h>

PlayerBullet::PlayerBullet() = default;

void PlayerBullet::Initialize(const Vector3& position)
{
	// ColliderにIDをセット
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kPlayerBullet));

	// 初期化
	isAlive_   = true;
	lifeFrame_ = PlayerBulletDefaults::kLifeFrames;

	worldTransform_.Initialize();
	worldTransform_.SetScale(PlayerBulletDefaults::kInitScale);
	worldTransform_.SetRotate(PlayerBulletDefaults::kInitRotate);
	worldTransform_.SetTranslate(position);

	// 弾オブジェクト
	objectBullet_ = std::make_unique<Object3d>();
	objectBullet_->Initialize("player_bullet.obj");
	objectBullet_->SetScale(worldTransform_.GetScale());
	objectBullet_->SetTranslate(worldTransform_.GetTranslate());

	// 初速を設定（右方向に飛ぶ例：X正方向）
	velocity_ = { kSpeed_, 0.0f, 0.0f };

	// 半径を設定
	SetRadius(radius_);
}

void PlayerBullet::Update()
{
	// 移動
	Move();

	// 寿命
	if (lifeFrame_ > 0) {
		--lifeFrame_;
	} else {
		isAlive_ = false;
	}

	// 変換更新と反映
	worldTransform_.Update();
	objectBullet_->SetScale(worldTransform_.GetScale());
	objectBullet_->SetTranslate(worldTransform_.GetTranslate());
	objectBullet_->Update();
}

void PlayerBullet::Draw()
{
	if (!isAlive_) return;
	objectBullet_->Draw();
}

void PlayerBullet::Move()
{
	// 速度に基づいて位置更新
	worldTransform_.SetTranslate(worldTransform_.GetTranslate() + velocity_);
}

void PlayerBullet::OnCollision(Collider* other)
{
	if (!isAlive_) return;

	// 敵や敵弾に当たったら消える（必要に応じて条件拡張）
	const uint32_t type = other->GetTypeID();
	if (type == static_cast<uint32_t>(CollisionTypeIdDef::kEnemy) ||
		type == static_cast<uint32_t>(CollisionTypeIdDef::kEnemyBullet)) {
		isAlive_ = false;
		SetRadius(0.0f); // 直ちに当たり判定無効化
	}
}

Vector3 PlayerBullet::GetCenterPosition() const
{
	return worldTransform_.GetTranslate();
}

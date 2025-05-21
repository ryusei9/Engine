#include "PlayerBullet.h"
#include <CollisionTypeIdDef.h>
#include <Player.h>
PlayerBullet::PlayerBullet()
{
	// シリアルナンバーを設定
	serialNumber_ = nextSerialNumber_;

	// 次のシリアルナンバーを設定
	nextSerialNumber_++;
}

void PlayerBullet::Initialize()
{
	// IDの設定
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kPlayerBullet));

	isAlive_ = true;
	lifeFrame_ = 180;

	// 座標の設定
	worldTransform_.scale = { 1.0f, 1.0f, 1.0f };
	worldTransform_.rotate = { 0.0f, 0.0f, 0.0f };
	worldTransform_.translate = { 0.0f, 0.0f, 0.0f };

	// オブジェクトの生成・初期化
	objectBullet_ = std::make_unique<Object3d>();
	objectBullet_->Initialize("player_bullet.obj");
	objectBullet_->SetScale(worldTransform_.scale);
	SetRadius(0.05f);
}

void PlayerBullet::Update()
{
	// プレイヤー弾の移動
	Move();

	// プレイヤー弾の生存フレームを減少
	// 生存フレームの更新
	if (lifeFrame_ > 0) {
		lifeFrame_--;
	} else {
		isAlive_ = false;
	}
	objectBullet_->Update();
	objectBullet_->SetTranslate(worldTransform_.translate);
	
}

void PlayerBullet::Draw()
{
	objectBullet_->Draw();
}

void PlayerBullet::Move()
{
	// プレイヤー弾の移動
	worldTransform_.translate.x += speed_;
	
}

void PlayerBullet::OnCollision(Collider* other)
{// プレイヤー弾の衝突判定
	if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeIdDef::kEnemy)) {
		// 敵と衝突した場合
		isAlive_ = false;
	}
}

Vector3 PlayerBullet::GetCenterPosition() const
{
	return worldTransform_.translate;
}

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

void PlayerBullet::Initialize(const Vector3& position)
{
	// IDの設定
 	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kPlayerBullet));

	isAlive_ = true;
	lifeFrame_ = 180;
	worldTransform_.Initialize();
	// 座標の設定
	worldTransform_.scale_ = { 1.0f, 1.0f, 1.0f };
	worldTransform_.rotate_ = { 0.0f, 0.0f, 0.0f };
	worldTransform_.translate_ = position;

	// オブジェクトの生成・初期化
	objectBullet_ = std::make_unique<Object3d>();
	objectBullet_->Initialize("player_bullet.obj");
	objectBullet_->SetScale(worldTransform_.scale_);
	objectBullet_->SetTranslate(worldTransform_.translate_);
	//objectBullet_->SetScale(worldTransform_.scale_);
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
	worldTransform_.Update();
	//objectBullet_->SetScale(worldTransform_.scale_);
	objectBullet_->SetTranslate(worldTransform_.translate_);
	objectBullet_->Update();
	
	
}

void PlayerBullet::Draw()
{
	objectBullet_->Draw();
}

void PlayerBullet::Move()
{
	// プレイヤー弾の移動
	worldTransform_.translate_.x += speed_;
	
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
	return worldTransform_.translate_;
}

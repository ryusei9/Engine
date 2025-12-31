#include "PlayerChargeBullet.h"
#include <CollisionTypeIdDef.h>

// 静的メンバ変数の初期化
uint32_t PlayerChargeBullet::sNextSerialNumber_ = PlayerChargeBulletDefaults::kSerialStart;

PlayerChargeBullet::PlayerChargeBullet()
{
	// シリアルナンバーを設定
	serialNumber_ = sNextSerialNumber_++;
}

void PlayerChargeBullet::Initialize(const Vector3& position)
{
	// 基底の初期化（位置・Transformなど）
	PlayerBullet::Initialize(position);

	// チャージ弾のコライダーID
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kPlayerChargeBullet));

	// 見た目のスケールを拡大（定数で倍率指定）
	if (objectBullet_) {
		const Vector3 scaled = worldTransform_.GetScale() * PlayerChargeBulletDefaults::kScaleFactor;
		objectBullet_->SetScale(scaled);
	}

	// 当たり判定の半径も拡大
	SetRadius(PlayerChargeBulletDefaults::kRadius);
}

void PlayerChargeBullet::Update()
{
	// 共通の更新（移動・寿命・描画更新）
	PlayerBullet::Update();
	// 必要ならチャージ弾専用の挙動をここに追加
}

void PlayerChargeBullet::Draw()
{
	// チャージ弾専用の描画があれば追加
	PlayerBullet::Draw();
}
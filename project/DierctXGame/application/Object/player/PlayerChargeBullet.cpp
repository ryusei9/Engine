#include "PlayerChargeBullet.h"
#include <CollisionTypeIdDef.h>

// 静的メンバ変数の初期化
uint32_t PlayerChargeBullet::sNextSerialNumber_ = 1;

PlayerChargeBullet::PlayerChargeBullet() {
	// シリアルナンバーを設定
    serialNumber_ = sNextSerialNumber_++;
}

void PlayerChargeBullet::Initialize(const Vector3& position) {
    PlayerBullet::Initialize(position);
    // コライダーのIDを設定
    Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kPlayerChargeBullet));
    // サイズを大きくする
    if (objectBullet_) {
        objectBullet_->SetScale({ 10.0f, 10.0f, 10.0f }); // 通常弾の2倍
    }
    SetRadius(0.5f); // 半径も大きく
}

void PlayerChargeBullet::Update() {
    PlayerBullet::Update();
    // 必要ならチャージ弾専用の挙動を追加
}

void PlayerChargeBullet::Draw() {
	// チャージ弾専用の描画処理があれば追加
    PlayerBullet::Draw();
}
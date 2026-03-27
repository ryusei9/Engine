#include "MiniBoss.h"
#include <Object3dCommon.h>
#include <CollisionTypeIdDef.h>
#include <PlayerBullet.h>
#include <PlayerChargeBullet.h>

using namespace MyEngine;

MiniBoss::MiniBoss()
    : worldTransform(BaseCharacter::GetWorldTransform())
{
    // シリアルナンバーを振る
    serialNumber_ = sNextSerialNumber_;
    // 次のシリアルナンバーに1を足す
	++sNextSerialNumber_;
}

void MiniBoss::Initialize()
{
    BaseCharacter::Initialize();

    // ワールド変換
    GetWorldTransform().Initialize();
    SetScale({ 3.0f,3.0f,3.0f });
    SetPosition({ 0.0f,0.0f,30.0f });

    // モデル
    object3d_ = std::make_unique<Object3d>();
    object3d_->Initialize("miniBoss.obj");

	object3d_->SetMaterialColor({ 1.0f, 0.0f, 0.0f, 1.0f });

    // コライダー
    SetRadius(radius_);
    Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kEnemy));

    // HP
    hp_ = 100;
}

void MiniBoss::Update()
{
    if (state_ == State::Dead) {
        return;
    }

    Move();
    Attack();

    BaseCharacter::Update();

    if (hp_ <= 0) {
        state_ = State::Dead;
        SetIsAlive(false);
    }
}

void MiniBoss::Draw()
{
    if (state_ == State::Alive) {
        BaseCharacter::Draw();
    }
}

void MiniBoss::Move()
{
    // 後で実装
}

void MiniBoss::Attack()
{
    // 後で実装
}

void MiniBoss::OnCollision(Collider* other)
{
    if (state_ != State::Alive) {
        return;
    }

    // プレイヤー弾
    if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeIdDef::kPlayerBullet)) {
        hp_ -= 1;
    }

    // チャージ弾
    if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeIdDef::kPlayerChargeBullet)) {
        hp_ -= 5;
    }
}

Vector3 MiniBoss::GetCenterPosition() const
{
    return GetWorldTransform().GetTranslate();
}
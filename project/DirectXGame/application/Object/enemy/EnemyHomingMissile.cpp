#include "EnemyHomingMissile.h"
#include <cmath>
#include <CollisionTypeIdDef.h>

void EnemyHomingMissile::Initialize(
    const Vector3& pos,
    const Vector3& velocity,
    Player* player,
    const std::string& paramFile)
{
    EnemyBullet::Initialize(pos, velocity, paramFile);
    player_ = player;

    Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kEnemyMissile));

    // 初速を一定速度に揃える（重要）
    Vector3 vel = GetVelocity();

    float len = std::sqrt(vel.x * vel.x + vel.y * vel.y + vel.z * vel.z);
    if (len > 0.001f) {
        vel.x /= len;
        vel.y /= len;
        vel.z /= len;
    }

    vel *= speed_;
    SetVelocity(vel);
}

void EnemyHomingMissile::Update()
{
    timer_ += 1.0f / 60.0f;

    // 追尾処理
    if (timer_ < homingTime_ && player_) {

        Vector3 pos = GetWorldTransform().GetTranslate();
        Vector3 toPlayer = player_->GetCenterPosition() - pos;

        float len = std::sqrt(
            toPlayer.x * toPlayer.x +
            toPlayer.y * toPlayer.y +
            toPlayer.z * toPlayer.z);

        if (len > 0.001f) {

            // 方向ベクトル（正規化）
            Vector3 dir{
                toPlayer.x / len,
                toPlayer.y / len,
                0.0f
            };

            Vector3 vel = GetVelocity();

            // 向きを徐々に変える
            vel.x += (dir.x - vel.x) * rotateSpeed_;
            vel.y += (dir.y - vel.y) * rotateSpeed_;

            // ★ここが超重要：速度を一定にする
            float vlen = std::sqrt(
                vel.x * vel.x +
                vel.y * vel.y +
                vel.z * vel.z);

            if (vlen > 0.001f) {
                vel.x /= vlen;
                vel.y /= vlen;
                vel.z /= vlen;
            }

            vel *= speed_;

            SetVelocity(vel);
        }
    }

    // 通常更新
    EnemyBullet::Update();
}

void EnemyHomingMissile::OnCollision(Collider* other)
{
    if (!IsAlive()) {
        return;
    }

    // プレイヤーの弾で破壊
    if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeIdDef::kPlayerBullet)) {
        SetAlive(false);
        SetRadius(0.0f);
        return;
    }

    // プレイヤー本体に当たった場合は共通処理
    EnemyBullet::OnCollision(other);
}

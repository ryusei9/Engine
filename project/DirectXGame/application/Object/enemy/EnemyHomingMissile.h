#pragma once
#include "EnemyBullet.h"
#include "Player.h"

class EnemyHomingMissile : public EnemyBullet
{
public:

    void Initialize(
        const Vector3& pos,
        const Vector3& velocity,
        Player* player,
        const std::string& paramFile);

    void Update() override;

    void OnCollision(Collider* other) override;

private:

    Player* player_ = nullptr;

    float homingTime_ = 3.0f;   // 追尾時間
    float timer_ = 0.0f;

    float rotateSpeed_ = 0.05f; // 旋回力

    float speed_ = 0.02f; // 好きな速さに調整
};
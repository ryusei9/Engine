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

    // Getter
    Player* GetPlayer() const { return player_; }
    float GetHomingTime() const { return homingTime_; }
    float GetTimer() const { return timer_; }
    float GetRotateSpeed() const { return rotateSpeed_; }
    float GetSpeed() const { return speed_; }

    // Setter
    void SetPlayer(Player* player) { player_ = player; }
    void SetHomingTime(float homingTime) { homingTime_ = homingTime; }
    void SetTimer(float timer) { timer_ = timer; }
    void SetRotateSpeed(float rotateSpeed) { rotateSpeed_ = rotateSpeed; }
    void SetSpeed(float speed) { speed_ = speed; }

private:

    Player* player_ = nullptr;

    float homingTime_ = 3.0f;   // 追尾時間
    float timer_ = 0.0f;

    float rotateSpeed_ = 0.05f; // 旋回力

    float speed_ = 0.03f; // 好きな速さに調整
};
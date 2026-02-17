#pragma once
#include <vector>
#include <cstdint>
#include "Vector3.h"

class Enemy;
class GamePlayScene;

class PlayerChargeBullet
{
public:
    PlayerChargeBullet();

    // 初期化（発動時）
    void Initialize(
        const Vector3& playerPosition,
        const std::vector<Enemy*>& enemies,
        GamePlayScene* scene);

    // 更新
    void Update(float deltaTime);

    // 描画（必要なら）
    void Draw();

    bool IsDead() const { return isDead_; }

private:

    // ロック対象収集
    void CollectTargets(
        const std::vector<Enemy*>& enemies,
        GamePlayScene* scene);

    // ダメージ処理
    void ApplyDamage(Enemy* enemy, float damage);

    // レーザー演出
    void EmitLaserTo(Enemy* enemy);

private:

    // ==== パラメータ ====
    float baseDamage_ = 60.0f;
    float maxActiveTime_ = 2.0f;
    uint32_t maxLockCount_ = 5;

    // ==== 状態 ====
    Vector3 playerPosition_;
    float activeTime_ = 0.0f;
    bool isDead_ = false;

    std::vector<Enemy*> targets_;
};

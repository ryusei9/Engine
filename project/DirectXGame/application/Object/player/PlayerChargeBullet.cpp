#include "PlayerChargeBullet.h"
#include "Enemy.h"
#include "GamePlayScene.h"
#include "ParticleManager.h"
#include <algorithm>
#include <cmath>
#include <Normalize.h>
#include <Length.h>

PlayerChargeBullet::PlayerChargeBullet()
{
}

void PlayerChargeBullet::Initialize(
    const Vector3& playerPosition,
    const std::vector<Enemy*>& enemies,
    GamePlayScene* scene)
{
    playerPosition_ = playerPosition;
    activeTime_ = 0.0f;
    isDead_ = false;
    targets_.clear();

    CollectTargets(enemies, scene);
}

void PlayerChargeBullet::CollectTargets(
    const std::vector<Enemy*>& enemies,
    GamePlayScene* scene)
{
    for (Enemy* enemy : enemies)
    {
        if (!enemy) continue;
        if (!enemy->IsAlive()) continue;

        if (scene->IsInCameraView(enemy->GetPosition()))
        {
            targets_.push_back(enemy);
        }
    }

    if (targets_.size() > maxLockCount_)
    {
        targets_.resize(maxLockCount_);
    }
}

void PlayerChargeBullet::Update(float deltaTime)
{
    if (isDead_) return;

    activeTime_ += deltaTime;

    if (activeTime_ >= maxActiveTime_)
    {
        isDead_ = true;
        return;
    }

    // 生きている敵のみ残す
    targets_.erase(
        std::remove_if(
            targets_.begin(),
            targets_.end(),
            [](Enemy* e) { return (!e || !e->IsAlive()); }),
        targets_.end());

    if (targets_.empty())
    {
        isDead_ = true;
        return;
    }

    float damagePerTarget =
        baseDamage_ / static_cast<float>(targets_.size());

    for (Enemy* enemy : targets_)
    {
        ApplyDamage(enemy, damagePerTarget);
        EmitLaserTo(enemy);
    }
}

void PlayerChargeBullet::ApplyDamage(
    Enemy* enemy,
    float damage)
{
    if (!enemy) return;

    // 継続ダメージ型
    enemy->TakeDamage(damage * 0.016f);
    // ↑ 60FPS前提で秒間換算
}

void PlayerChargeBullet::EmitLaserTo(Enemy* enemy)
{
    if (!enemy) return;

    Vector3 start = playerPosition_;
    Vector3 end = enemy->GetPosition();

    Vector3 direction = end - start;
    float length = Length::Length(direction);
    direction = Normalize::Normalize(direction);

    const int division = 8;
    float step = length / static_cast<float>(division);

    ParticleManager* pm = ParticleManager::GetInstance();

    for (int i = 0; i < division; ++i)
    {
        Vector3 pos = start + direction * (step * i);
        pm->Emit("Laser", pos, 1);
    }
}

void PlayerChargeBullet::Draw()
{
    // 必要ならここに追加描画
}

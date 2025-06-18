#pragma once
#include <memory>
#include <vector>
#include <EnemyBullet.h>
#include <Player.h>
#include <imgui.h>

class Enemy;

// ビヘイビア基底クラス
class EnemyAttackPattern {
public:
    virtual ~EnemyAttackPattern() = default;
    virtual void Update(Enemy* enemy, Player* player, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) = 0;
    virtual void DrawImGui(int idx, bool selected) = 0;
    virtual const char* GetName() const = 0;
private:
	WorldTransform worldTransform_; // ワールド変換
};

// パターン1: 画面右側で上下移動しつつ扇形弾
class EnemyAttackPatternFan : public EnemyAttackPattern {
public:
    void Update(Enemy* enemy, Player* player, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) override;
    void DrawImGui(int idx, bool selected) override;
    const char* GetName() const override { return "Pattern1"; }
private:
    float moveDir_ = 1.0f;
    float moveTimer_ = 0.0f;
    float shotTimer_ = 0.0f;
};

// パターン2: 右側中央で自機狙い弾連射
class EnemyAttackPatternAimed : public EnemyAttackPattern {
public:
    void Update(Enemy* enemy, Player* player, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) override;
    void DrawImGui(int idx, bool selected) override;
    const char* GetName() const override { return "Pattern2"; }
private:
    float shotTimer_ = 0.0f;
};

// パターン3: 全方位弾+左突進→左で消えて右から復活
class EnemyAttackPatternRush : public EnemyAttackPattern {
public:
    void Update(Enemy* enemy, Player* player, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) override;
    void DrawImGui(int idx, bool selected) override;
    const char* GetName() const override { return "Pattern3"; }
private:
    float shotTimer_ = 0.0f;
    bool rushing_ = false;
};

class EnemyAttackPatternWait : public EnemyAttackPattern {
public:
    void Update(Enemy* enemy, Player*, std::list<std::unique_ptr<EnemyBullet>>&, float deltaTime) override;
    void DrawImGui(int idx, bool selected) override;
    const char* GetName() const override { return "wait"; }
private:
    bool easing_ = false;
    float easingTime_ = 0.0f;
    float easingDuration_ = 2.0f;
    Vector3 startPos_;
};

class EnemyAttack {
public:
    EnemyAttack();
    void Update(Enemy* enemy, Player* player, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime);
    void DrawImGui();
    void SetPattern(int idx);
    int GetPattern() const { return currentPattern_; }
    void SetNextPattern(int idx) { nextPattern_ = idx; }
private:
    std::vector<std::unique_ptr<EnemyAttackPattern>> patterns_;
    int currentPattern_ = 0;
    int nextPattern_ = -1;
    float patternTimer_ = 0.0f;
    bool pattern3Rushed_ = false;
};
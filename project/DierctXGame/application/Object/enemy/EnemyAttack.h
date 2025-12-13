#pragma once
#include <memory>
#include <vector>
#include <EnemyBullet.h>
#include <Player.h>
#include <imgui.h>

class Enemy;
/// <summary>
/// ビヘイビア基底クラス
/// </summary>
class EnemyAttackPattern {
public:
	// デストラクタ
    virtual ~EnemyAttackPattern() = default;

	// 更新
    virtual void Update(Enemy* enemy, Player* player, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) = 0;

	// ImGui描画
    virtual void DrawImGui(int32_t idx, bool selected) = 0;

	// パターン名取得
    virtual const char* GetName() const = 0;
private:
	WorldTransform worldTransform_; // ワールド変換
};

// パターン1: 画面右側で上下移動しつつ扇形弾
class EnemyAttackPatternFan : public EnemyAttackPattern {
public:
	// 更新
    void Update(Enemy* enemy, Player* player, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) override;

	// ImGui描画
    void DrawImGui(int32_t idx, bool selected) override;

	// パターン名取得
    const char* GetName() const override { return "Pattern1"; }
private:
    float moveDir_ = 1.0f;
    float moveTimer_ = 0.0f;
    float shotTimer_ = 0.0f;
};

// パターン2: 右側中央で自機狙い弾連射
class EnemyAttackPatternAimed : public EnemyAttackPattern {
public:
	// 更新
    void Update(Enemy* enemy, Player* player, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) override;

	// ImGui描画
    void DrawImGui(int32_t idx, bool selected) override;

	// パターン名取得
    const char* GetName() const override { return "Pattern2"; }
private:
    float shotTimer_ = 0.0f;
};

// パターン3: 全方位弾+左突進→左で消えて右から復活
class EnemyAttackPatternRush : public EnemyAttackPattern {
public:
	// 更新
    void Update(Enemy* enemy, Player* player, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) override;

	// ImGui描画
    void DrawImGui(int32_t idx, bool selected) override;

	// パターン名取得
    const char* GetName() const override { return "Pattern3"; }
private:
    float shotTimer_ = 0.0f;
    bool rushing_ = false;
};

// パターン4: 待機（イージング移動）
class EnemyAttackPatternWait : public EnemyAttackPattern {
public:
	// 更新
    void Update(Enemy* enemy, Player*, std::list<std::unique_ptr<EnemyBullet>>&, float deltaTime) override;

	// ImGui描画
    void DrawImGui(int32_t idx, bool selected) override;

	// パターン名取得
    const char* GetName() const override { return "wait"; }
private:
    bool easing_ = false;
    float easingTime_ = 0.0f;
    float easingDuration_ = 2.0f;
    Vector3 startPos_;
};


/// <summary>
/// 敵攻撃管理クラス
/// </summary>
class EnemyAttack {
public:
	// コンストラクタ
    EnemyAttack();

	// 更新
    void Update(Enemy* enemy, Player* player, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime);

	// ImGui描画
    void DrawImGui();

	// パターン設定・取得
    void SetPattern(int32_t idx);

	// 現在のパターン取得
    int32_t GetPattern() const { return currentPattern_; }

	// 次のパターン設定
    void SetNextPattern(int32_t idx) { nextPattern_ = idx; }
private:
    std::vector<std::unique_ptr<EnemyAttackPattern>> patterns_;
    int32_t currentPattern_ = 0;
    int32_t nextPattern_ = -1;
    float patternTimer_ = 0.0f;
    bool pattern3Rushed_ = false;
};
#pragma once
#include <memory>
#include <vector>
#include <EnemyBullet.h>
#include <Player.h>
#include <imgui.h>

class Enemy;

/// <summary>
/// 敵攻撃の調整用定数（マジックナンバー排除）
/// </summary>
namespace EnemyAttackDefaults {
	// 共通
	inline constexpr float kPi = 3.1415926535f;
	inline constexpr float kDelta60Hz = 1.0f / 60.0f;

	// Pattern1: 扇形
	inline constexpr float kFanMoveSpeedY = 0.02f;
	inline constexpr float kFanMoveRangeY = 2.0f;
	inline constexpr float kFanShotIntervalSec = 2.0f;
	inline constexpr int32_t kFanShotCount = 5;
	inline constexpr float kFanBaseAngle = kPi;          // 左向き
	inline constexpr float kFanSpread = kPi / 3.0f;      // 扇の広がり
	inline constexpr float kFanBulletSpeed = 0.15f;

	// Pattern2: 自機狙い
	inline constexpr float kAimedShotIntervalSec = 2.0f;
	inline constexpr float kAimedBulletSpeed = 0.18f;
	inline constexpr float kBulletSpeedScale = 0.5f;     // 速度スケール
	inline constexpr float kAimedMinLen = 0.01f;

	// Pattern3: 突進＋全方位
	inline constexpr float kRushStartX = 5.0f;
	inline constexpr float kRushSpeedX = 0.08f;
	inline constexpr float kRushShotIntervalSec = 0.3f;
	inline constexpr int32_t kRushRingCount = 12;
	inline constexpr float kRushRingSpeed = 0.13f;
	inline constexpr float kRushLeftEndX = -4.0f;
	inline constexpr float kRushRespawnX = 4.0f;
	inline constexpr float kRushResetX = 3.0f;

	// Pattern遷移
	inline constexpr float kPattern1DurationSec = 10.0f;

	// Pattern4: 待機（イージング）
	inline constexpr float kWaitEasingDurationSec = 2.0f;
}

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

// パターン4: 待機（イージング）
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
    float easingDuration_ = EnemyAttackDefaults::kWaitEasingDurationSec;
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
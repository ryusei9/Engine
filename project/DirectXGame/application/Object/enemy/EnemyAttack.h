#pragma once
#include <memory>
#include <vector>
#include <list>
#include <cstdint>
#include <EnemyBullet.h>
#include <Player.h>
#include <EnemyHomingMissile.h>

// 前方宣言
class Enemy;
struct Vector3;

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
	inline constexpr float kFanBaseAngle = kPi;
	inline constexpr float kFanSpread = kPi / 3.0f;
	inline constexpr float kFanBulletSpeed = 0.15f;

	// Pattern2: 自機狙い
	inline constexpr float kAimedShotIntervalSec = 2.0f;
	inline constexpr float kAimedBulletSpeed = 0.18f;
	inline constexpr float kBulletSpeedScale = 0.5f;
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

	// Pattern4: 待機
	inline constexpr float kWaitEasingDurationSec = 2.0f;
}

/// <summary>
/// 敵攻撃のパラメータ構造体（JSONから読み込み）
/// </summary>
struct EnemyAttackParameters {
	// --- Pattern1: 扇形 ---
	/// <summary>扇形攻撃のY軸移動速度</summary>
	float fanMoveSpeedY = EnemyAttackDefaults::kFanMoveSpeedY;

	/// <summary>扇形攻撃のY軸移動範囲</summary>
	float fanMoveRangeY = EnemyAttackDefaults::kFanMoveRangeY;

	/// <summary>扇形攻撃の発射間隔（秒）</summary>
	float fanShotIntervalSec = EnemyAttackDefaults::kFanShotIntervalSec;

	/// <summary>扇形攻撃の同時発射数</summary>
	int32_t fanShotCount = EnemyAttackDefaults::kFanShotCount;

	/// <summary>扇形攻撃の基準角度（ラジアン）</summary>
	float fanBaseAngle = EnemyAttackDefaults::kFanBaseAngle;

	/// <summary>扇形攻撃の広がり角度（ラジアン）</summary>
	float fanSpread = EnemyAttackDefaults::kFanSpread;

	/// <summary>扇形攻撃の弾速</summary>
	float fanBulletSpeed = EnemyAttackDefaults::kFanBulletSpeed;

	// --- Pattern2: 自機狙い ---
	/// <summary>自機狙い攻撃の発射間隔（秒）</summary>
	float aimedShotIntervalSec = EnemyAttackDefaults::kAimedShotIntervalSec;

	/// <summary>自機狙い攻撃の弾速</summary>
	float aimedBulletSpeed = EnemyAttackDefaults::kAimedBulletSpeed;

	/// <summary>速度のスケール倍率</summary>
	float bulletSpeedScale = EnemyAttackDefaults::kBulletSpeedScale;

	/// <summary>自機狙いの最小距離</summary>
	float aimedMinLen = EnemyAttackDefaults::kAimedMinLen;

	// --- Pattern3: 突進＋全方位 ---
	/// <summary>突進攻撃の開始X座標</summary>
	float rushStartX = EnemyAttackDefaults::kRushStartX;

	/// <summary>突進攻撃のX軸速度</summary>
	float rushSpeedX = EnemyAttackDefaults::kRushSpeedX;

	/// <summary>突進中の全方位弾発射間隔（秒）</summary>
	float rushShotIntervalSec = EnemyAttackDefaults::kRushShotIntervalSec;

	/// <summary>全方位弾の同時発射数</summary>
	int32_t rushRingCount = EnemyAttackDefaults::kRushRingCount;

	/// <summary>全方位弾の弾速</summary>
	float rushRingSpeed = EnemyAttackDefaults::kRushRingSpeed;

	/// <summary>突進の左端（リスポーン判定位置）X座標</summary>
	float rushLeftEndX = EnemyAttackDefaults::kRushLeftEndX;

	/// <summary>突進後のリスポーン先X座標</summary>
	float rushRespawnX = EnemyAttackDefaults::kRushRespawnX;

	/// <summary>突進完了後の定位置X座標</summary>
	float rushResetX = EnemyAttackDefaults::kRushResetX;

	// --- Pattern遷移 ---
	/// <summary>Pattern1の継続時間（秒）</summary>
	float pattern1DurationSec = EnemyAttackDefaults::kPattern1DurationSec;

	// --- Pattern4: 待機 ---
	/// <summary>待機イージングの継続時間（秒）</summary>
	float waitEasingDurationSec = EnemyAttackDefaults::kWaitEasingDurationSec;
};

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

	// パラメータ設定
	virtual void SetParameters(const EnemyAttackParameters& params) { parameters_ = params; }

protected:
	// パラメータ
	EnemyAttackParameters parameters_;
};

/// <summary>
/// パターン1: 画面右側で上下移動しつつ扇形弾
/// </summary>
class EnemyAttackPatternFan : public EnemyAttackPattern {
public:
	// 更新
	void Update(Enemy* enemy, Player* player, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) override;

	// ImGui描画
	void DrawImGui(int32_t idx, bool selected) override;

	// パターン名取得
	const char* GetName() const override { return "Pattern1:Fan"; }

private:
	float moveDir_ = 1.0f;
	float shotTimer_ = 0.0f;
};

/// <summary>
/// パターン2: 右側中央で自機狙い弾連射
/// </summary>
class EnemyAttackPatternAimed : public EnemyAttackPattern {
public:
	// 更新
	void Update(Enemy* enemy, Player* player, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) override;

	// ImGui描画
	void DrawImGui(int32_t idx, bool selected) override;

	// パターン名取得
	const char* GetName() const override { return "Pattern2:Aimed"; }

private:
	float shotTimer_ = 0.0f;
};

/// <summary>
/// パターン3: 全方位弾+左突進→左で消えて右から復活
/// </summary>
class EnemyAttackPatternRush : public EnemyAttackPattern {
public:
	// 更新
	void Update(Enemy* enemy, Player* player, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) override;

	// ImGui描画
	void DrawImGui(int32_t idx, bool selected) override;

	// パターン名取得
	const char* GetName() const override { return "Pattern3:Rush"; }

private:
	float shotTimer_ = 0.0f;
	bool rushing_ = false;
};

/// <summary>
/// パターン4: 待機
/// </summary>
class EnemyAttackPatternWait : public EnemyAttackPattern {
public:
	// 更新
	void Update(Enemy* enemy, Player*, std::list<std::unique_ptr<EnemyBullet>>&, float deltaTime) override;

	// ImGui描画
	void DrawImGui(int32_t idx, bool selected) override;

	// パターン名取得
	const char* GetName() const override { return "Pattern4:Wait"; }
};

/// <summary>
/// パターン5: ミサイル
/// </summary>

class EnemyAttackPatternMissile : public EnemyAttackPattern {
public:

	void Update(
		Enemy* enemy,
		Player* player,
		std::list<std::unique_ptr<EnemyBullet>>& bullets,
		float deltaTime) override;

	void DrawImGui(int32_t idx, bool selected) override;

	const char* GetName() const override { return "Pattern5:Missile"; }

private:

	float shotTimer_ = 0.0f;
};

/// <summary>
/// 敵攻撃管理クラス
/// </summary>
class EnemyAttack {
public:
	// コンストラクタ
	EnemyAttack();

	// パラメータファイルから初期化
	void Initialize(const std::string& parameterFileName);

	// 更新
	void Update(Enemy* enemy, Player* player, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime);

	// ImGui描画
	void DrawImGui();

	// パターン設定・取得
	void SetPattern(int32_t idx);

	// 現在のパターン取得
	int32_t GetPattern() const { return currentPattern_; }

	// パラメータ設定
	void SetParameters(const EnemyAttackParameters& params);

	// パラメータ取得
	const EnemyAttackParameters& GetParameters() const { return parameters_; }

private:
	// パラメータ
	EnemyAttackParameters parameters_;

	// パターンリスト
	std::vector<std::unique_ptr<EnemyAttackPattern>> patterns_;

	// 現在のパターン
	int32_t currentPattern_ = 0;

	// パターン遷移タイマー
	float patternTimer_ = 0.0f;

	// パターン3突進フラグ
	bool pattern3Rushed_ = false;
};
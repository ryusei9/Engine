#pragma once
#include "BaseCharacter.h"
#include <ParticleManager.h>
#include <ParticleEmitter.h>
#include <EnemyBullet.h>
#include "EnemyAttack.h"
#include <memory>
#include <list>
#include <cstdint>
#include <string>
#include <CurveMoveManager.h>
#include "LevelData.h" // EnemyMove と CurveData の定義

// 前方宣言
class Player;

/// <summary>
/// 敵キャラクターの調整用定数（マジックナンバー排除）
/// </summary>
namespace EnemyDefaults {
	inline constexpr float kMoveSpeed        = 0.1f;
	inline constexpr int32_t kInitialHp      = 1;
	inline constexpr float kRespawnTimeSec   = 3.0f;
	inline constexpr float kColliderRadius   = 1.0f;
	inline constexpr float kShotIntervalSec  = 5.0f;   // 発射間隔
	inline constexpr float kDeathDurationSec = 1.0f;   // 落下演出 duration
	inline constexpr float kFallSpeed        = -0.02f; // 落下速度
	inline constexpr float kRotationSpeed    = 0.01f;  // 回転速度
	inline constexpr int32_t kInvalidColliderId = -1;
}

/// <summary>
/// 敵の調整パラメータ構造体（JSONから読み込み）
/// </summary>
struct EnemyParameters {
	// 基本パラメータ
	float moveSpeed = EnemyDefaults::kMoveSpeed;
	int32_t initialHp = EnemyDefaults::kInitialHp;
	float respawnTimeSec = EnemyDefaults::kRespawnTimeSec;
	float colliderRadius = EnemyDefaults::kColliderRadius;
	float shotIntervalSec = EnemyDefaults::kShotIntervalSec;
	
	// 死亡演出パラメータ
	float deathDurationSec = EnemyDefaults::kDeathDurationSec;
	float fallSpeed = EnemyDefaults::kFallSpeed;
	float rotationSpeed = EnemyDefaults::kRotationSpeed;
	
	// パーティクルパラメータ
	int smokeParticleRate = 60;
	int smokeParticleCount = 3;
	int deathParticleRate = 8;
	int deathParticleCount = 8;
	int hitParticleRate = 1;
	int hitParticleCount = 0;
	
	// 煙エフェクトパラメータ
	float smokePower = 2.0f;
	float smokePowerMultiplier = 0.1f;
	float smokeUpwardForce = 2.0f;
};

/// <summary>
/// 敵キャラクタークラス
/// </summary>
class Enemy : public BaseCharacter
{
public:

	/*------ステート------*/
	enum class EnemyState {
		Alive,
		Dying,
		Dead
	};

	/*------メンバ関数------*/

	// コンストラクタ
	Enemy();

	// 初期化
	void Initialize() override;

	// パラメータファイルから初期化
	void Initialize(const std::string& parameterFileName);

	// 更新
	void Update() override;

	// 描画
	void Draw() override;

	// ImGui描画
	void DrawImGui();

	// キャラクターの移動
	void Move() override;

	// キャラクターの攻撃
	void Attack() override;

	// 衝突判定
	void OnCollision(Collider* other) override;

	// 敵死亡時に一度だけパーティクルを出す
	void PlayDeathParticleOnce();

	// 敵ヒット時にパーティクルを出す
	void PlayHitParticle();

	void StartCurveMove(const CurveData& curve);

	/*------ゲッター------*/

	// 中心座標を取得する純粋仮想関数
	Vector3 GetCenterPosition() const override;

	// 敵の半径を取得
	float GetRadius() const { return radius_; }

	// プレイヤーへのポインタを取得
	Player* GetPlayer() const { return player_; }

	// 敵の弾リストを取得
	std::list<std::unique_ptr<EnemyBullet>>& GetBullets() { return bullets_; }

	// 敵のステートを取得
	EnemyState GetState() const { return state_; }

	// エネミー操作有効化フラグの取得
	bool GetControlEnabled() const { return controlEnabled_; }

	// コライダーIDの取得
	int GetColliderId() const { return colliderId_; }

	// パラメータの取得
	const EnemyParameters& GetParameters() const { return parameters_; }

	/*------セッター------*/

	// プレイヤーへのポインタを設定
	void SetPlayer(Player* player) { player_ = player; }

	// エネミー操作有効化フラグの設定
	void SetControlEnabled(bool enabled) { controlEnabled_ = enabled; } 

	// Z軸の座標を設定
	void SetZ(float z) { worldTransform_.translate_.z = z; }

	// パラメータの設定
	void SetParameters(const EnemyParameters& parameters);

	void SetMoveType(EnemyMove type) { moveType_ = type; }

private:
	/*------メンバ変数------*/

	// 敵の調整パラメータ（JSONから読み込み）
	EnemyParameters parameters_;

	// 敵の移動速度
	float moveSpeed_ = EnemyDefaults::kMoveSpeed;

	// 敵のヒットポイント
	int32_t hp_ = EnemyDefaults::kInitialHp;

	// 敵のシリアルナンバー
	uint32_t serialNumber_ = 0;

	// 敵のカメラ
	Camera* camera_ = nullptr;

	// 敵のリスポーンタイム
	float respawnTime_ = EnemyDefaults::kRespawnTimeSec;
	
	// 敵の半径
	float radius_ = EnemyDefaults::kColliderRadius;

	// パーティクルマネージャーへのポインタ
	ParticleManager* particleManager_ = nullptr;

	// 敵死亡時のパーティクルエミッター
	std::unique_ptr<ParticleEmitter> enemyDeathEmitter_;

	// 敵ヒット時のパーティクルエミッター
	std::unique_ptr<ParticleEmitter> enemyHitEmitter_;

	// 煙用のパーティクルエミッター
	std::unique_ptr<ParticleEmitter> smokeEmitter_;

	// 敵死亡時にパーティクルを一度だけ再生するためのフラグ
	bool hasPlayedDeathParticle_ = false;

	// 敵ヒット時にパーティクルを一度だけ再生するためのフラグ
	bool hasPlayedHitParticle_ = false;

	// 敵の弾リスト
	std::list<std::unique_ptr<EnemyBullet>> bullets_;

	// 発射間隔（秒）
	float shotInterval_ = EnemyDefaults::kShotIntervalSec;

	// 発射タイマー
	float shotTimer_ = 0.0f;

	// 敵の攻撃パターン
	std::unique_ptr<EnemyAttack> attack_;

	// プレイヤーへのポインタ
	Player* player_ = nullptr;

	// 演出用エネミーが動かないフラグ
	bool controlEnabled_ = false;

	// コライダーID
	int32_t colliderId_ = EnemyDefaults::kInvalidColliderId;

	// ステート用変数
	EnemyState state_ = EnemyState::Alive;

	// 落下演出 duration
	float deathTimer_ = EnemyDefaults::kDeathDurationSec;

	// 落下速度
	float fallSpeed_ = EnemyDefaults::kFallSpeed;

	// 回転速度
	float rotationSpeed_ = EnemyDefaults::kRotationSpeed;

	// 死亡エフェクト再生フラグ
	bool deathEffectPlayed_ = false;

	// カーブ移動マネージャー
	std::unique_ptr<CurveMoveManager> curveMoveManager_;

	// 敵の動きパターン
	EnemyMove moveType_ = EnemyMove::None;
};


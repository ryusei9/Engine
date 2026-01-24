#pragma once
#include "BaseCharacter.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"
#include <memory>
#include <list>
#include <string>
#include <cstdint>

// 前方宣言
class PlayerBullet;
class PlayerChargeBullet;
struct Vector3;

/// <summary>
/// プレイヤー調整用定数（マジックナンバー排除）
/// </summary>
namespace PlayerDefaults {
	inline constexpr float kMoveSpeed         = 0.1f;
	inline constexpr float kRadius            = 0.1f;
	inline constexpr float kRespawnWaitSec    = 2.0f;
	inline constexpr float kDelta60Hz         = 1.0f / 60.0f;

	// チャージ関連
	inline constexpr float kChargeReadySec    = 3.0f;

	// パーティクル
	inline constexpr uint32_t kThrusterRate   = 60;
	inline constexpr uint32_t kThrusterCount  = 3;
	inline constexpr float    kThrusterOffsetX = 0.2f;
	inline constexpr float    kExplosionRate  = 1.0f;
	inline constexpr uint32_t kExplosionCount = 0;

	// 初期Transform
	inline constexpr Vector3  kInitScale  = { 1.0f, 1.0f, 1.0f };
	inline constexpr Vector3  kInitRotate = { 0.0f, 0.0f, 0.0f };
	inline constexpr Vector3  kInitTranslate = { 0.0f, 0.0f, 0.0f };
}

/// <summary>
/// プレイヤーのパラメータ構造体（JSONから読み込み）
/// </summary>
struct PlayerParameters {
	// 基本パラメータ
	float moveSpeed = PlayerDefaults::kMoveSpeed;
	float radius = PlayerDefaults::kRadius;
	float respawnWaitSec = PlayerDefaults::kRespawnWaitSec;

	// チャージ関連
	float chargeReadySec = PlayerDefaults::kChargeReadySec;

	// パーティクルパラメータ
	uint32_t thrusterRate = PlayerDefaults::kThrusterRate;
	uint32_t thrusterCount = PlayerDefaults::kThrusterCount;
	float thrusterOffsetX = PlayerDefaults::kThrusterOffsetX;
	float explosionRate = PlayerDefaults::kExplosionRate;
	uint32_t explosionCount = PlayerDefaults::kExplosionCount;

	// スラスター計算パラメータ
	float thrusterBasePower = 2.0f;
	float thrusterVelocityMultiplier = 1.5f;

	// 初期Transform
	Vector3 initScale = PlayerDefaults::kInitScale;
	Vector3 initRotate = PlayerDefaults::kInitRotate;
	Vector3 initTranslate = PlayerDefaults::kInitTranslate;

	// モデルファイル名
	std::string modelFileName = "player.obj";

	// パーティクルテクスチャファイル名
	std::string thrusterTexture = "resources/circle2.png";
	std::string explosionTexture = "resources/circle2.png";
};

/// <summary>
/// プレイヤークラス
/// </summary>
class Player : public BaseCharacter
{
public:
	/*------メンバ関数------*/

	// コンストラクタ
	Player();

	// 初期化
	void Initialize() override;

	// パラメータファイルから初期化
	void Initialize(const std::string& parameterFileName);

	// 更新
	void Update() override;

	// 描画
	void Draw() override;

	// キャラクターの移動
	void Move() override;

	// キャラクターの攻撃
	void Attack() override;

	// 衝突判定
	void OnCollision(Collider* other) override;

	// ImGuiでの描画
	void DrawImGui();

	// プレイヤー死亡時に一度だけパーティクルを出す
	void PlayDeathParticleOnce();

	// 中心座標を取得する純粋仮想関数
	Vector3 GetCenterPosition() const override;

	// 速度ベクトルを取得
	const Vector3& GetVelocity() const { return velocity_; }

	/*------ゲッター------*/

	// プレイヤーの弾を取得
	std::list<std::unique_ptr<PlayerBullet>>& GetBullets() { return bullets_; }

	// プレイヤーのチャージ弾を取得
	std::list<std::unique_ptr<PlayerChargeBullet>>& GetChargeBullets() { return chargeBullets_; }

	// プレイヤーのコントロール有効フラグを取得
	bool GetPlayerControlEnabled() const { return controlEnabled_; }

	// プレイヤーの生存状態を取得
	bool GetIsAlive() const { return isAlive_; }

	// パラメータを取得
	const PlayerParameters& GetParameters() const { return parameters_; }

	/*------セッター------*/

	// プレイヤーのコントロール有効フラグを設定
	void SetPlayerControlEnabled(bool enabled) { controlEnabled_ = enabled; }

	// パラメータを設定
	void SetParameters(const PlayerParameters& parameters);

private:
	/*------メンバ変数------*/

	// パラメータ（JSONから読み込み）
	PlayerParameters parameters_;

	// 武器
	std::list<std::unique_ptr<PlayerBullet>> bullets_;

	// チャージ弾
	std::list<std::unique_ptr<PlayerChargeBullet>> chargeBullets_;

	// プレイヤーの移動速度
	float moveSpeed_ = PlayerDefaults::kMoveSpeed;

	// 速度ベクトル
	Vector3 velocity_ = { 0.0f, 0.0f, 0.0f };
	
	// 半径
	float radius_ = PlayerDefaults::kRadius;
	
	// リスポーンタイマー
	float respawnTimer_ = 0.0f;

	// プレイヤーのシリアルナンバー
	uint32_t serialNumber_ = 0;
	
	// プレイヤーのカメラ
	Camera* camera_ = nullptr;

	// 発射フラグ
	bool isShot_ = false;

	// パーティクルマネージャー
	ParticleManager* particleManager_ = nullptr;

	// スラスターパーティクルエミッター
	std::unique_ptr<ParticleEmitter> thrusterEmitter_;

	// 爆発パーティクルエミッター
	std::unique_ptr<ParticleEmitter> explosionEmitter_;

	// プレイヤー死亡時にパーティクルを一度だけ再生するためのフラグ
	bool hasPlayedDeathParticle_ = false;

	// チャージショット関連
	float chargeTime_ = 0.0f;
	bool isCharging_ = false;
	bool chargeReady_ = false;

	// 演出用プレイヤーが動かないフラグ
	bool controlEnabled_ = true;
};


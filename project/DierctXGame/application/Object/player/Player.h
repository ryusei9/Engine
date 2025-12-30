#pragma once
#include "BaseCharacter.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"

class PlayerBullet;        // 前方宣言
class PlayerChargeBullet;  // 前方宣言

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
	inline constexpr uint32_t kThrusterRate   = 60;     // 1秒間に60個
	inline constexpr uint32_t kThrusterCount  = 3;
	inline constexpr float    kThrusterOffsetX = 0.2f;  // 左オフセット
	inline constexpr float    kExplosionRate  = 1.0f;
	inline constexpr uint32_t kExplosionCount = 0;

	// 初期Transform
	inline constexpr Vector3  kInitScale  = { 1.0f, 1.0f, 1.0f };
	inline constexpr Vector3  kInitRotate = { 0.0f, 0.0f, 0.0f };
	inline constexpr Vector3  kInitTranslate = { 0.0f, 0.0f, 0.0f };
}

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

	bool GetPlayerControlEnabled() const { return controlEnabled_; }

	// プレイヤーの死亡フラグの取得
	bool GetIsAlive() const { return isAlive_; }

	/*------セッター------*/
	void SetPlayerControlEnabled(bool enabled) { controlEnabled_ = enabled; }

private:
	/*------メンバ変数------*/

	std::list<std::unique_ptr<PlayerBullet>>        bullets_;        // 武器
	std::list<std::unique_ptr<PlayerChargeBullet>>  chargeBullets_;  // チャージ弾

	// プレイヤーの移動速度
	float moveSpeed_ = PlayerDefaults::kMoveSpeed;

	Vector3 velocity_ = { 0.0f, 0.0f, 0.0f }; // 速度ベクトル
	
	float radius_ = PlayerDefaults::kRadius; // 半径
	
	// リスポーンタイマー
	float respawnTimer_ = 0.0f;

	// プレイヤーのシリアルナンバー
	uint32_t serialNumber_ = 0;
	
	// プレイヤーのカメラ
	Camera* camera_ = nullptr;

	bool isShot_ = false; // 発射フラグ

	ParticleManager* particleManager_ = nullptr; // パーティクルマネージャー

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


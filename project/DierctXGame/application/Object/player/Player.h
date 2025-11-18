#pragma once
#include "BaseCharacter.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"

class PlayerBullet; // 前方宣言
class PlayerChargeBullet; // 前方宣言

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
	void PlayDeathParticleOnce(); // 追加: 一度だけパーティクルを出す関数

	// 中心座標を取得する純粋仮想関数
	Vector3 GetCenterPosition() const override;

	// 速度ベクトルを取得
	const Vector3& GetVelocity() const { return velocity_; }

	/*------ゲッター------*/

	// プレイヤーの弾を取得
	std::list<std::unique_ptr<PlayerBullet>>& GetBullets() { return bullets_; }

	// プレイヤーのチャージ弾を取得
	std::list<std::unique_ptr<PlayerChargeBullet>>& GetChargeBullets() { return chargeBullets_; }

	bool GetPlayerControlEnabled() const { return controlEnabled_; } // プレイヤー操作有効化フラグの取得

	// プレイヤーの死亡フラグの取得
	bool GetIsAlive() const { return isAlive_; }

	/*------セッター------*/
	void SetPlayerControlEnabled(bool enabled) { controlEnabled_ = enabled; } // プレイヤー操作有効化フラグの設定

private:
	/*------メンバ変数------*/

	std::list<std::unique_ptr<PlayerBullet>> bullets_; // 武器
	std::list<std::unique_ptr<PlayerChargeBullet>> chargeBullets_; // チャージ弾

	// プレイヤーの移動速度
	float moveSpeed_ = 0.1f;

	Vector3 velocity_ = { 0.0f, 0.0f, 0.0f }; // 速度ベクトルを追加
	
	float radius_ = 0.1f; // 半径
	
	float respawnTimer_ = 0.0f;
	// プレイヤーのシリアルナンバー
	uint32_t serialNumber_ = 0;
	// プレイヤーのワールド変換
	//Transform worldTransform_;
	// プレイヤーのカメラ
	Camera* camera_ = nullptr;

	bool isShot_ = false; // 発射フラグ

	ParticleManager* particleManager_; // パーティクルマネージャー

	// メンバ変数
	std::unique_ptr<ParticleEmitter> thrusterEmitter_;

	std::unique_ptr<ParticleEmitter> explosionEmitter_;

	bool hasPlayedDeathParticle_ = false;

	float chargeTime_ = 0.0f;
	bool isCharging_ = false;
	bool chargeReady_ = false;

	// 演出用プレイヤーが動かないフラグ
	bool controlEnabled_ = true;
};


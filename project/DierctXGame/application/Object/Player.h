#pragma once
#include "BaseCharacter.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"

class PlayerBullet; // 前方宣言
class Player : public BaseCharacter
{
public:
	/*------メンバ関数------*/

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

	void PlayDeathParticleOnce(); // 追加: 一度だけパーティクルを出す関数

	// 中心座標を取得する純粋仮想関数
	Vector3 GetCenterPosition() const override;

	const Vector3& GetVelocity() const { return velocity_; }

	/*------ゲッター------*/
	std::list<std::unique_ptr<PlayerBullet>>& GetBullets() { return bullets_; }

	//void SetBullet(PlayerBullet* bullet) { bullet_ = bullet; } // 武器の設定

	//void SetAttack(bool attack) { isAttack_ = attack; } // 攻撃フラグの設定

private:
	/*------メンバ変数------*/

	std::list<std::unique_ptr<PlayerBullet>> bullets_; // 武器
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
};


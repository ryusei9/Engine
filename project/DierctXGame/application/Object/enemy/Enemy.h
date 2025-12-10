#pragma once
#include "BaseCharacter.h"
#include <ParticleManager.h>
#include <ParticleEmitter.h>
#include <EnemyBullet.h>
#include "EnemyAttack.h"
class Player; // 前方宣言

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

	/*------ゲッター------*/

	// 中心座標を取得する純粋仮想関数
	Vector3 GetCenterPosition() const override;

	// 敵の半径を取得
	float GetRadius() const { return radius_; }

	// プレイヤーへのポインタを取得
	Player* GetPlayer() const { return player_; } // プレイヤーへのポインタを取得

	// 敵の弾リストを取得
	std::list<std::unique_ptr<EnemyBullet>>& GetBullets() { return bullets_; }

	// 敵のステートを取得
	EnemyState GetState() const { return state_; }

	/*------セッター------*/

	// プレイヤーへのポインタを設定
	void SetPlayer(Player* player) { player_ = player; }

	// エネミー操作有効化フラグの取得
	bool GetControlEnabled() const { return controlEnabled_; }

	// エネミー操作有効化フラグの設定
	void SetControlEnabled(bool enabled) { controlEnabled_ = enabled; } 

	// コライダーIDの取得
	int GetColliderId() const { return colliderId_; }

	// Z軸の座標を設定
	void SetZ(float z) {worldTransform_.translate_.z = z;}

	

private:
	/*------メンバ変数------*/
	// 敵の移動速度
	float moveSpeed_ = 0.1f;
	// 敵のヒットポイント
	int hp_ = 1;
	// 敵のシリアルナンバー
	uint32_t serialNumber_ = 0;
	// 敵のワールド変換
	//Transform worldTransform_;
	// 敵のカメラ
	Camera* camera_ = nullptr;

	// 敵のリスポーンタイム
	float respawnTime_ = 3.0f;
	
	// 敵の半径
	float radius_ = 1.0f;

	// パーティクルマネージャーへのポインタ
	ParticleManager* particleManager = nullptr;

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
	float shotInterval_ = 5.0f; // 1秒ごとに発射
	float shotTimer_ = 0.0f;

	// 敵の攻撃パターン
	std::unique_ptr<EnemyAttack> attack_;

	// プレイヤーへのポインタ
	Player* player_ = nullptr;

	// 演出用エネミーが動かないフラグ
	bool controlEnabled_ = false;

	// コライダーID
	int colliderId_ = -1;

	// ステート用変数
	EnemyState state_ = EnemyState::Alive;

	// 死亡演出用変数
	float deathTimer_ = 1.0f;      // 落下演出 duration
	float fallSpeed_ = -0.02f;     // 落下速度
	float rotationSpeed_ = 0.01f;   // 回転速度
	bool deathEffectPlayed_ = false;
};


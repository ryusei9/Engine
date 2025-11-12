#pragma once
#include "BaseCharacter.h"
#include <ParticleManager.h>
#include <ParticleEmitter.h>
#include <EnemyBullet.h>
#include "EnemyAttack.h"
class Player; // 前方宣言
class Enemy : public BaseCharacter
{
public:
	/*------メンバ関数------*/
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

	void PlayDeathParticleOnce(); // 追加: 一度だけパーティクルを出す関数


	// 中心座標を取得する純粋仮想関数
	Vector3 GetCenterPosition() const override;

	// 敵の半径を取得
	float GetRadius() const { return radius_; }

	Player* GetPlayer() const { return player_; } // プレイヤーへのポインタを取得

	void SetPlayer(Player* player) { player_ = player; } // プレイヤーへのポインタを設定

	std::list<std::unique_ptr<EnemyBullet>>& GetBullets() { return bullets_; }

	bool GetControlEnabled() const { return controlEnabled_; } // エネミー操作有効化フラグの取得

	void SetControlEnabled(bool enabled) { controlEnabled_ = enabled; } // エネミー操作有効化フラグの設定

	int GetColliderId() const { return colliderId_; }

	Vector3 GetInitialPosition() const {return initialPosition_;}

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
	

	float radius_ = 1.0f;

	ParticleManager* particleManager = nullptr;

	std::unique_ptr<ParticleEmitter> enemyDeathEmitter_; // 敵死亡時のパーティクルエミッター

	bool hasPlayedDeathParticle_ = false;

	std::list<std::unique_ptr<EnemyBullet>> bullets_;
	float shotInterval_ = 5.0f; // 1秒ごとに発射
	float shotTimer_ = 0.0f;

	std::unique_ptr<EnemyAttack> attack_;

	Player* player_ = nullptr; // プレイヤーへのポインタ

	// 演出用エネミーが動かないフラグ
	bool controlEnabled_ = false;

	int colliderId_ = -1;

	Vector3 initialPosition_ = { 0.0f, 0.0f, 0.0f };
};


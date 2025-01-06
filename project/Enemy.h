#pragma once
#include "Object3d.h"
#include <Transform.h>
#include <EnemyBullet.h>

class Player;

class GameScene;

class Enemy
{
public:
	/// <summary>
	///  デストラクタ
	/// </summary>
	~Enemy();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Object3d* model,Object3d* bulletModel, const Vector3& position);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 接近フェーズの更新
	/// </summary>
	void ApproachPheseUpdate();

	/// <summary>
	/// 離脱フェーズの更新
	/// </summary>
	void BattlePheseUpdate();

	/// <summary>
	/// 弾発射
	/// </summary>
	void Fire();

	// 発射間隔
	static const int kFireInterval = 60;

	// 接近フェーズ初期化
	void ApproachPheseInitialize();

	void SetPlayer(Player* player) { player_ = player; }

	// ワールド座標を取得
	Vector3 GetWorldPosition();

	// 衝突を検出したら呼び出されるコールバック関数
	void OnCollision();

	// 弾リストを取得
	const std::list<EnemyBullet*>& GetBullets() const { return bullets_; }

	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }

	bool IsDead() const { return isDead_; }

	

	void SetInitialize(const Vector3& position) {
		isDead_ = false;
		hp_ = 30;
		transform_.translate = position;
		transform_.scale = { 3.0f,3.0f,3.0f };
		transform_.rotate = { 0.0f,-0.0f,0.0f };
		fireTimer = 300;
		phese_ = Phese::Approach;
		ApproachPheseInitialize();
	}

private:
	// ワールド変換データ
	Transform transform_;

	// モデル
	Object3d* model_ = nullptr;

	// テクスチャハンドル
	uint32_t textureHandle_ = 0u;

	
	// 行動フェーズ
	enum class Phese {
		Approach,	// 接近する
		Battle,		// 離脱する
	};

	// フェーズ
	Phese phese_ = Phese::Approach;

	// 弾
	std::list<EnemyBullet*> bullets_;

	Object3d* bulletModel_ = nullptr;

	// 発射タイマー
	int32_t fireTimer = 0;

	// 自キャラ
	Player* player_ = nullptr;

	GameScene* gameScene_ = nullptr;

	// 体力
	int32_t hp_ = 10;

	// デスフラグ
	bool isDead_ = false;
};


#pragma once
#include "Model.h"
#include "WorldTransform.h"
#include "MathMatrix.h"
#include "EnemyBullet.h"
class GameScene;
// 自機クラスの前方宣言
class Player;
/// <summary>
/// 敵
/// </summary>
class Enemy {
public:
	/// <summary>
	///  デストラクタ
	/// </summary>
	~Enemy();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Model* model, const Vector3& position);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw(const ViewProjection& viewProjection);

	/// <summary>
	/// 接近フェーズの更新
	/// </summary>
	void ApproachPheseUpdate();

	/// <summary>
	/// 離脱フェーズの更新
	/// </summary>
	void LeavePheseUpdate();

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
	//const std::list<EnemyBullet*>& GetBullets() const { return bullets_; }

	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }

	bool IsDead() const { return isDead_; }

private:
	// ワールド変換データ
	WorldTransform worldTransform_;

	// モデル
	Model* model_ = nullptr;

	// テクスチャハンドル
	uint32_t textureHandle_ = 0u;

	// 数学関数
	MathMatrix* mathMatrix_ = nullptr;

	// 行動フェーズ
	enum class Phese {
		Approach,	// 接近する
		Leave,		// 離脱する
	};

	// フェーズ
	Phese phese_ = Phese::Approach;

	// 弾
	//std::list<EnemyBullet*> bullets_;

	// 発射タイマー
	int32_t fireTimer = 0;

	// 自キャラ
	Player* player_ = nullptr;

	GameScene* gameScene_ = nullptr;

	// デスフラグ
	bool isDead_ = false;
};

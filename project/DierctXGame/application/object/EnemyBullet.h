#pragma once
#include "Object3d.h"
#include "Transform.h"
class EnemyBullet
{
public:
	/// <summary>
	///  デストラクタ
	/// </summary>
	~EnemyBullet();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Object3d* model, const Vector3& position, const Vector3& velocity);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	bool IsDead() const { return isDead_; }

	// ワールド座標を取得
	Vector3 GetWorldPosition();

	// 衝突を検出したら呼び出されるコールバック関数
	void OnCollision();

	

private:
	// ワールド変換データ
	Transform transform_;

	// モデル
	Object3d* model_ = nullptr;

	// テクスチャハンドル
	uint32_t textureHandle_ = 0u;

	// 速度
	Vector3 velocity_;

	// 寿命<frm>
	static const int32_t kLifeTime = 60 * 5;

	// デスタイマー
	int32_t deathTimer_ = kLifeTime;
	// デスフラグ
	bool isDead_ = false;
};


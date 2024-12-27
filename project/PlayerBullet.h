#pragma once
#include <memory>
#include <Vector3.h>
#include <Object3d.h>
#include <Transform.h>
class PlayerBullet
{
public:
	/// <summary>
	///  デストラクタ
	/// </summary>
	~PlayerBullet();
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

	void OnCollision();

	// ワールド座標を取得
	Vector3 GetWorldPosition();


private:
	
	// ワールド変換データ
	Transform transform_;

	//モデル
	//std::shared_ptr<BaseModel> model_;

	//オブジェクト
	Object3d* model_;

	// テクスチャハンドル
	//uint32_t textureHandle_ = 0u;

	// 速度
	Vector3 velocity_;

	// 寿命<frm>
	static const int32_t kLifeTime = 60 * 5;

	// デスタイマー
	int32_t deathTimer_ = kLifeTime;
	// デスフラグ
	bool isDead_ = false;
};


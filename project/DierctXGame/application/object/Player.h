#pragma once
#include "Vector3.h"
#include "Input.h"
#include "Object3d.h"
#include <PlayerBullet.h>
#include <Transform.h>
class Player
{
public:
	~Player();
	/// メンバ関数
	/// <summary>
	///  初期化
	/// </summary>
	void Initialize(Object3d* model,Object3d* bulletModel);

	/// <summary>
	///  更新
	/// </summary>
	void Update();

	/// <summary>
	///  描画
	/// </summary>
	void Draw();

	// 弾リストを取得
	const std::list<PlayerBullet*>& GetBullets() const { return bullets_; }

	void Attack();

	// ワールド座標を取得
	//Vector3 GetWorldPosition();

	// 衝突を検出したら呼び出されるコールバック関数
	void OnCollision();

	/// <summary>
	/// ワールド座標を取得
	/// </summary>
	/// <returns></returns>
	Vector3 GetWorldPosition();

	// 死亡フラグのゲッター
	bool IsDead() { return isDead_; }

	/// <summary>
	/// 半径を取得
	/// </summary>
	/// <returns></returns>
	float GetRadius()const { return radius_; }
private:
	
	Transform transform;

	Input* input_;

	Object3d* model_;


	// 弾丸のモデルを生成
	Object3d* bulletModel_;

	// 弾
	std::list<PlayerBullet*> bullets_;

	// 弾の発射地点
	Transform bulletEmitter{};


	// クールタイム（秒）
	const float kCoolDownTime = 15.0f;

	float fireCoolTime = 0.0f;

	// プレイヤーの死亡フラグ
	bool isDead_;

	// 当たり判定の大きさ（半径）
	float radius_ = 1.0f;
};


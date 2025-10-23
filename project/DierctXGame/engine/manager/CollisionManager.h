#pragma once
#include <list>
#include <memory>
#include "Vector3.h"
class Collider;// 前方宣言

/// <summary>
///	当たり判定管理クラス
/// </summary>
class CollisionManager
{
public:
	/*------メンバ関数------*/

	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	// リセット
	void Reset();

	// 衝突判定を行う
	void CheckCollision();

	// 衝突オブジェクトを追加
	void AddCollider(Collider* collider);

	// 衝突オブジェクトを削除
	void RemoveCollider(Collider* collider);

	// コライダー2つの衝突判定と応答処理
	void CheckCollisionPair(Collider* colliderA, Collider* colliderB);

	// 球体同士の衝突判定
	bool CheckSphereCollision(Collider* colliderA, Collider* colliderB);

private:
	/*------メンバ変数------*/

	// 衝突オブジェクトのリスト
	std::list<Collider*> colliders_;
};


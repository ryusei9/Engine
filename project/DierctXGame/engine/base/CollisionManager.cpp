#include "CollisionManager.h"
#include "Collider.h"

void CollisionManager::Initialize()
{

}

void CollisionManager::Update()
{
	// 更新処理
	for (Collider* collider : colliders_)
	{
		collider->Update();
	}
}

void CollisionManager::Draw()
{
	// 描画処理
	for (Collider* collider : colliders_)
	{
		collider->Draw();
	}
}

void CollisionManager::Reset()
{
	colliders_.clear();
}

void CollisionManager::CheckCollision()
{
	// リスト内のペアを総当たり
	std::list<Collider*>::iterator itrA = colliders_.begin();
	for (; itrA != colliders_.end(); ++itrA)
	{
		Collider* colliderA = *itrA;

		// itrBはitrAの次の要素からまわす
		std::list<Collider*>::iterator itrB = itrA;
		itrB++;

		for (; itrB != colliders_.end(); ++itrB)
		{
			Collider* colliderB = *itrB;

			// ペアの当たり判定
			CheckCollisionPair(colliderA, colliderB);
		}
	}
}

void CollisionManager::AddCollider(Collider* collider)
{
	colliders_.push_back(collider);
}

void CollisionManager::RemoveCollider(Collider* collider)
{
	colliders_.remove(collider);
}

void CollisionManager::CheckCollisionPair(Collider* colliderA, Collider* colliderB)
{
	// 衝突判定
	if (CheckSphereCollision(colliderA, colliderB))
	{
		// 衝突したオブジェクトの情報を取得
		colliderA->OnCollision(colliderB);
		colliderB->OnCollision(colliderA);
	}
}

bool CollisionManager::CheckSphereCollision(Collider* colliderA, Collider* colliderB)
{
	// 中心座標を取得
	Vector3 centerA = colliderA->GetCenterPosition();
	Vector3 centerB = colliderB->GetCenterPosition();
	// 半径を取得
	float radiusA = colliderA->GetRadius();
	float radiusB = colliderB->GetRadius();
	// 距離を計算
	Vector3 distance = centerA - centerB;
	float distanceSquared = Vector3::Length(distance);
	// 半径の和を計算
	float radiusSum = radiusA + radiusB;
	
	// 衝突判定
	return distanceSquared <= radiusSum;
}

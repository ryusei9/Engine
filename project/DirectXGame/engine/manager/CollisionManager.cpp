#include "CollisionManager.h"
#include "Collider.h"
#include "CollisionTypeIdDef.h"
#include <cmath>

//
// CollisionManager
// - 簡易衝突管理クラス。
// - 役割：シーン内の Collider ポインタ群を保持し、各フレームで総当たりにより衝突判定を行う。
// - 設計方針：
//   * シンプルな実装のためナイーブな O(n^2) の全探索を行う（小規模オブジェクト数を想定）。
//   * 衝突の判定は球（Sphere）による簡易判定を用いる。
//   * 衝突発生時は各 Collider の OnCollision(Collider*) を呼び出して応答させる（コールバック方式）。
// - 注意点 / 制約：
//   * colliders_ は生ポインタのリストを保持している（所有権は外部が持つ想定）。登録解除やライフサイクル管理は呼び出し側で行うこと。
//   * 大量のコライダーが存在する場合は Broad-phase（空間分割 / BVH / グリッド等）を導入して性能改善を検討すること。
//   * CheckSphereCollision 内では距離計算に sqrt を用いて実際の距離を比較している（最適化の余地あり：距離の二乗を比較する方法を推奨）。
//   * 衝突ペアのフィルタリングは CheckCollisionPair 内で行っている。新しいタイプ追加時はここにルールを追加する必要がある。
//

using namespace CollisionManagerConstants;

void CollisionManager::Initialize()
{
	// 初期化処理（現状は特になし）
	// 将来的に空間分割データ構造等を初期化する場合はここに実装する
	colliders_.clear();
}

void CollisionManager::Update()
{
	// 各コライダーの個別更新（位置や状態の同期など）
	for (Collider* collider : colliders_) {
		if (collider) {
			collider->Update();
		}
	}
}

void CollisionManager::Draw()
{
	// デバッグ表示など、コライダー自体の描画（可視化）を行う
	for (Collider* collider : colliders_) {
		if (collider) {
			collider->Draw();
		}
	}
}

void CollisionManager::Reset()
{
	// 登録されているコライダー一覧をクリアする
	// - コライダー自体の破棄は行わない（ownershipは外部）
	colliders_.clear();
}

void CollisionManager::CheckCollision()
{
	// 全体の衝突判定ルーチン（全ペア総当たり）
	CheckAllPairs();
}

void CollisionManager::AddCollider(Collider* collider)
{
	// nullptrチェック
	if (!collider) return;
	
	// コライダーを登録する
	colliders_.push_back(collider);
}

void CollisionManager::RemoveCollider(Collider* collider)
{
	// nullptrチェック
	if (!collider) return;
	
	// コライダーを登録リストから削除する
	colliders_.remove(collider);
}

void CollisionManager::CheckCollisionPair(Collider* colliderA, Collider* colliderB)
{
	// nullptrチェック
	if (!colliderA || !colliderB) return;

	// タイプIDを取得
	uint32_t typeA = colliderA->GetTypeID();
	uint32_t typeB = colliderB->GetTypeID();

	// フィルタリング: 衝突判定が不要な組み合わせをスキップ
	if (!ShouldCheckCollision(typeA, typeB)) {
		return;
	}

	// 実際の衝突判定
	bool isColliding = kUseSquaredDistance 
		? CheckSphereCollisionOptimized(colliderA, colliderB)
		: CheckSphereCollision(colliderA, colliderB);

	// 衝突検出時に通知
	if (isColliding) {
		NotifyCollision(colliderA, colliderB);
	}
}

bool CollisionManager::CheckSphereCollision(Collider* colliderA, Collider* colliderB)
{
	// nullptrチェック
	if (!colliderA || !colliderB) return false;

	// 球（Sphere）同士の当たり判定（実距離を使用）
	Vector3 centerA = colliderA->GetCenterPosition();
	Vector3 centerB = colliderB->GetCenterPosition();
	float radiusA = colliderA->GetRadius();
	float radiusB = colliderB->GetRadius();

	// 中心間のベクトル差
	Vector3 diff = centerA - centerB;
	
	// 距離を計算
	float distance = Vector3::Length(diff);
	
	// 半径の和
	float radiusSum = radiusA + radiusB;
	
	// 衝突判定
	return distance <= radiusSum;
}

// ===== ヘルパー関数 =====

bool CollisionManager::ShouldCheckCollision(uint32_t typeA, uint32_t typeB) const
{
	// フィルタリング例：
	// - 敵弾同士は当たらない
	// - 敵 と 敵弾 の組み合わせも不要
	uint32_t enemyBullet = static_cast<uint32_t>(CollisionTypeIdDef::kEnemyBullet);
	uint32_t enemy = static_cast<uint32_t>(CollisionTypeIdDef::kEnemy);

	// 敵弾同士
	if (typeA == enemyBullet && typeB == enemyBullet) {
		return false;
	}

	// 敵と敵弾（双方向）
	if ((typeA == enemy && typeB == enemyBullet) || 
	    (typeA == enemyBullet && typeB == enemy)) {
		return false;
	}

	return true;
}

bool CollisionManager::CheckSphereCollisionOptimized(Collider* colliderA, Collider* colliderB) const
{
	// nullptrチェック
	if (!colliderA || !colliderB) return false;

	// 球（Sphere）同士の当たり判定（最適化版：二乗距離を使用）
	Vector3 centerA = colliderA->GetCenterPosition();
	Vector3 centerB = colliderB->GetCenterPosition();
	float radiusA = colliderA->GetRadius();
	float radiusB = colliderB->GetRadius();

	// 中心間のベクトル差
	Vector3 diff = centerA - centerB;
	
	// 距離の二乗を計算（sqrtを回避）
	float distanceSquared = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
	
	// 半径の和の二乗
	float radiusSum = radiusA + radiusB;
	float radiusSumSquared = radiusSum * radiusSum;
	
	// 衝突判定（二乗距離で比較）
	return distanceSquared <= radiusSumSquared;
}

void CollisionManager::NotifyCollision(Collider* colliderA, Collider* colliderB)
{
	// 衝突検出時にコールバックを通知する
	// 注意: OnCollision 内でコライダーの削除が行われるとイテレーションに影響を与える可能性があるため、
	// 必要に応じて削除は別途マーク方式にするなどの対策を検討すること
	
	if (colliderA) {
		colliderA->OnCollision(colliderB);
	}
	
	if (colliderB) {
		colliderB->OnCollision(colliderA);
	}
}

void CollisionManager::CheckAllPairs()
{
	// 実装: 二重ループでペアを生成。重複チェックを避けるため itrB は itrA の次から開始する。
	// 時間計算量は O(n^2)。要素数が増えると性能悪化するため注意。
	
	std::list<Collider*>::iterator itrA = colliders_.begin();
	for (; itrA != colliders_.end(); ++itrA) {
		Collider* colliderA = *itrA;
		
		// nullptrチェック
		if (!colliderA) continue;

		// itrB は itrA の次から回す（同一ペアの二重処理を回避）
		std::list<Collider*>::iterator itrB = itrA;
		++itrB;

		for (; itrB != colliders_.end(); ++itrB) {
			Collider* colliderB = *itrB;
			
			// nullptrチェック
			if (!colliderB) continue;

			// ペアの当たり判定（内部でフィルタリングを行う）
			CheckCollisionPair(colliderA, colliderB);
		}
	}
}

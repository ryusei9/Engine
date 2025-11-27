#include "CollisionManager.h"
#include "Collider.h"
#include "CollisionTypeIdDef.h"

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


void CollisionManager::Initialize()
{
	// 初期化処理（現状は特になし）
	// 将来的に空間分割データ構造等を初期化する場合はここに実装する
}

void CollisionManager::Update()
{
	// 各コライダーの個別更新（位置や状態の同期など）
	// - ここで各 Collider::Update() を呼び、内部の transform 等を更新させる
	for (Collider* collider : colliders_)
	{
		collider->Update();
	}
}

void CollisionManager::Draw()
{
	// デバッグ表示など、コライダー自体の描画（可視化）を行う
	for (Collider* collider : colliders_)
	{
		collider->Draw();
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
	// 実装: 二重ループでペアを生成。重複チェックを避けるため itrB は itrA の次から開始する。
	// 時間計算量は O(n^2)。要素数が増えると性能悪化するため注意。
	std::list<Collider*>::iterator itrA = colliders_.begin();
	for (; itrA != colliders_.end(); ++itrA)
	{
		Collider* colliderA = *itrA;

		// itrB は itrA の次から回す（同一ペアの二重処理を回避）
		std::list<Collider*>::iterator itrB = itrA;
		itrB++;

		for (; itrB != colliders_.end(); ++itrB)
		{
			Collider* colliderB = *itrB;

			// ペアの当たり判定（内部でフィルタリングを行う）
			CheckCollisionPair(colliderA, colliderB);
		}
	}
}

void CollisionManager::AddCollider(Collider* collider)
{
	// コライダーを登録する（呼び出し側でライフタイムを管理する設計）
	colliders_.push_back(collider);
}

void CollisionManager::RemoveCollider(Collider* collider)
{
	// コライダーを登録リストから削除する
	colliders_.remove(collider);
}

void CollisionManager::CheckCollisionPair(Collider* colliderA, Collider* colliderB)
{
	// ペアごとの衝突判定エントリポイント
	// - ここでタイプ別の判定スキップ（フィルタ）を実施して不要な判定を省く
	// - 実際の衝突判定は CheckSphereCollision に委譲し、衝突時に双方へ OnCollision を通知する

	// タイプIDを取得
	uint32_t typeA = colliderA->GetTypeID();
	uint32_t typeB = colliderB->GetTypeID();

	// フィルタリング例：
	// - 敵弾同士は当たらない
	// - 敵 と 敵弾 の組み合わせも不要（双方向で確認しているので両パターンを排除）
	if ((typeA == static_cast<uint32_t>(CollisionTypeIdDef::kEnemyBullet) && typeB == static_cast<uint32_t>(CollisionTypeIdDef::kEnemyBullet)) ||
		(typeA == static_cast<uint32_t>(CollisionTypeIdDef::kEnemy) && typeB == static_cast<uint32_t>(CollisionTypeIdDef::kEnemyBullet)) ||
		(typeA == static_cast<uint32_t>(CollisionTypeIdDef::kEnemyBullet) && typeB == static_cast<uint32_t>(CollisionTypeIdDef::kEnemy))) {
		return;
	}
	// 実際の判定
	if (CheckSphereCollision(colliderA, colliderB))
	{
		// 衝突検出時にコールバックを通知する
		// 注意: OnCollision 内でコライダーの削除が行われるとイテレーションに影響を与える可能性があるため、
		// 必要に応じて削除は別途マーク方式にするなどの対策を検討すること
		colliderA->OnCollision(colliderB);
		colliderB->OnCollision(colliderA);
	}
}

bool CollisionManager::CheckSphereCollision(Collider* colliderA, Collider* colliderB)
{
	// 球（Sphere）同士の当たり判定
	// - 判定式: distance(centerA, centerB) <= radiusA + radiusB
	// - 現状は実距離を計算して比較している（sqrt を含む）。性能改善のためには二乗比較へ変更を推奨。
	//
	// 手順:
	// 1) 中心座標と半径を取得
	// 2) 中心間距離（実距離）を計算
	// 3) 半径和と比較して衝突判定を返す

	// 中心座標を取得
	Vector3 centerA = colliderA->GetCenterPosition();
	Vector3 centerB = colliderB->GetCenterPosition();
	// 半径を取得
	float radiusA = colliderA->GetRadius();
	float radiusB = colliderB->GetRadius();
	// 中心間のベクトル差
	Vector3 diff = centerA - centerB;
	// 距離を計算（Vector3::Length は実距離を返す実装を想定）
	float distance = Vector3::Length(diff);
	// 半径の和
	float radiusSum = radiusA + radiusB;
	
	// 衝突していれば true
	return distance <= radiusSum;
}

#pragma once
#include <list>
#include <memory>
#include "Vector3.h"

class Collider; // 前方宣言

// CollisionManager用の定数
namespace CollisionManagerConstants {
	// パフォーマンス警告用の閾値
	constexpr size_t kPerformanceWarningThreshold = 100;
	
	// 最適化のヒント: 距離の二乗比較を使用する
	constexpr bool kUseSquaredDistance = true;
}

/// <summary>
/// 当たり判定管理クラス
/// </summary>
class CollisionManager
{
public:
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

	// ゲッター
	size_t GetColliderCount() const { return colliders_.size(); }
	const std::list<Collider*>& GetColliders() const { return colliders_; }

private:
	// 衝突判定のフィルタリング
	bool ShouldCheckCollision(uint32_t typeA, uint32_t typeB) const;
	
	// 球体同士の衝突判定（最適化版：二乗距離を使用）
	bool CheckSphereCollisionOptimized(Collider* colliderA, Collider* colliderB) const;
	
	// 衝突応答の通知
	void NotifyCollision(Collider* colliderA, Collider* colliderB);
	
	// 全ペアの衝突判定を実行
	void CheckAllPairs();

	// 衝突オブジェクトのリスト
	std::list<Collider*> colliders_;
};


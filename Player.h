#pragma once
#include "Model.h"
#include "WorldTransform.h"
#include "Input.h"
#include "MathMatrix.h"
#include "PlayerBullet.h"
#include <list>
#include <Sprite.h>
#include <ViewProjection.h>

class Player {
public:

	/// <summary>
	///  デストラクタ
	/// </summary>
	~Player();
	
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Model* model, uint32_t textureHandle, Vector3 position, ViewProjection* viewProjection);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 旋回
	/// </summary>
	void Rotate();

	/// <summary>
	/// 攻撃
	/// </summary>
	void Attack();

	/// <summary>
	/// 親となるワールドトランスフォームをセット
	/// </summary>
	void SetParent(const WorldTransform* parent);

	// ワールド座標を取得
	Vector3 GetWorldPosition();

	Vector3 GetReticleWorldPosition();

	// 衝突を検出したら呼び出されるコールバック関数
	void OnCollision();

	// 弾リストを取得
	const std::list<PlayerBullet*>& GetBullets() const { return bullets_; }

	/// <summary>
	/// UI描画
	/// </summary>
	void DrawUI();

private:

	// ワールド変換データ
	WorldTransform worldTransform_;

	// モデル
	Model* model_ = nullptr;

	// テクスチャハンドル
	uint32_t textureHandle_ = 0u;

	// キーボード入力
	Input* input_ = nullptr;

	// 数学関数
	MathMatrix* mathMatrix_ = nullptr;

	// 可変個配列
	std::vector<std::vector<WorldTransform*>> worldTransformBlocks_;

	// 弾
	std::list<PlayerBullet*> bullets_;

	// 3Dレティクル用ワールドトランスフォーム
	WorldTransform worldTransform3DReticle_;

	// レティクルのモデル
	Model* reticleModel_ = nullptr;

	uint32_t reticleTextureHandle_ = 0u;

	// 2Dレティクル用スプライト
	Sprite* sprite2DReticle_ = nullptr;

	ViewProjection* viewProjection_;
};

#pragma once
#include <cstdint>
#include "Vector3.h"
class Collider
{
public:
	/*------メンバ関数------*/
	
	// デストラクタ
	virtual ~Collider() = default;

	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();
	
	// 衝突したオブジェクトの情報を取得
	virtual void OnCollision([[maybe_unused]] Collider* other) {}

	// 中心座標の取得
	virtual Vector3 GetCenterPosition() const = 0;

	/*------ゲッター------*/

	// 識別IDを取得
	uint32_t GetTypeID() const { return typeID_; }

	// 半径を取得
	float GetRadius() const { return radius_; }

	/*------セッター------*/
	// 識別IDを設定
	void SetTypeID(uint32_t typeID) { typeID_ = typeID; }

	// 半径を設定
	void SetRadius(float radius) { radius_ = radius; }

private:
	/*------メンバ変数------*/
	uint32_t typeID_ = 0; // 識別ID

	float radius_ = 1.0f; // 半径
};


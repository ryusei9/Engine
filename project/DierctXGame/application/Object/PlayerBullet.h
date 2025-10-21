#pragma once
#include "Collider.h"
#include <WorldTransform.h>
#include <memory>
#include <Object3d.h>

class Player; // 前方宣言
class PlayerBullet : public Collider
{
public:
	/*------メンバ関数------*/
	/// コンストラクタ
	PlayerBullet();

	// 初期化
	virtual void Initialize(const Vector3& position);

	// 更新
	virtual void Update();

	// 描画
	virtual void Draw();

	// プレイヤー弾の移動
	void Move();

	// 衝突判定
	void OnCollision(Collider* other) override;
	// 中心座標を取得する純粋仮想関数
	Vector3 GetCenterPosition() const override;

	/*------ゲッター------*/

	/*------セッター------*/
	// プレイヤーの設定
	void SetPlayer(Player* player) { player_ = player; }

	void SetTranslate(const Vector3& translate) { worldTransform_.translate_ = translate; } // 座標の設定

	bool IsAlive() const { return isAlive_; } // 生存フラグの取得

	float GetRadius() const { return radius_; } // 半径の取得


protected:
	/*------メンバ変数------*/
	WorldTransform worldTransform_;

	// プレイヤー
	Player* player_ = nullptr;

	// 弾のオブジェクト
	std::unique_ptr<Object3d> objectBullet_;

	// 速度
	Vector3 velocity_ = {};

	// 生存フラグ
	bool isAlive_ = true;
	// 生存フレーム
	uint32_t lifeFrame_ = 360;

	// 速度
	const float speed_ = 0.2f;

	// シリアルナンバー
	uint32_t serialNumber_ = 0;
	// 次のシリアルナンバー
	uint32_t nextSerialNumber_ = 0;

	float radius_ = 0.5f; // 半径
};


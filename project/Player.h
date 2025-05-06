#pragma once
#include "BaseCharacter.h"
#include <PlayerBullet.h>
class Player : public BaseCharacter
{
public:
	/*------メンバ関数------*/

	Player();
	// 初期化
	void Initialize() override;

	// 更新
	void Update() override;

	// 描画
	void Draw() override;

	// キャラクターの移動
	void Move() override;

	// キャラクターの攻撃
	void Attack() override;

	// 衝突判定
	void OnCollision(Collider* other) override;

	// 中心座標を取得する純粋仮想関数
	Vector3 GetCenterPosition() const override;

	void SetBullet(PlayerBullet* bullet) { bullet_ = bullet; } // 武器の設定

private:
	/*------メンバ変数------*/

	PlayerBullet* bullet_ = nullptr; // 武器
	// プレイヤーの移動速度
	float moveSpeed_ = 0.1f;
	
	// プレイヤーのヒットポイント
	int hp_ = 1;
	// プレイヤーのシリアルナンバー
	uint32_t serialNumber_ = 0;
	// プレイヤーのワールド変換
	Transform worldTransform_;
	// プレイヤーのカメラ
	Camera* camera_ = nullptr;
};


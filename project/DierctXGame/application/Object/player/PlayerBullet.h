#pragma once
#include "Collider.h"
#include <WorldTransform.h>
#include <memory>
#include <Object3d.h>

class Player; // 前方宣言

/// プレイヤー弾の調整用定数（マジックナンバー排除）
namespace PlayerBulletDefaults {
	inline constexpr uint32_t kLifeFrames   = 360;   // 生存フレーム
	inline constexpr float    kSpeed        = 0.2f;  // 移動速度
	inline constexpr float    kRadius       = 0.5f;  // 当たり半径
	inline constexpr Vector3  kInitScale    = { 1.0f, 1.0f, 1.0f };
	inline constexpr Vector3  kInitRotate   = { 0.0f, 0.0f, 0.0f };
}

/// <summary>
/// プレイヤーの弾
/// </summary>
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

	// 移動
	void Move();

	// 衝突判定
	void OnCollision(Collider* other) override;

	// 中心座標を取得する純粋仮想関数
	Vector3 GetCenterPosition() const override;

	/*------ゲッター------*/
	// 座標の取得（コピー回避）
	const Vector3& GetTranslate() const { return worldTransform_.GetTranslate(); }

	// 生存フラグの取得
	bool IsAlive() const { return isAlive_; }

	// 半径の取得
	float GetRadius() const { return radius_; }

	/*------セッター------*/
	// プレイヤーの設定
	void SetPlayer(Player* player) { player_ = player; }

	// 座標の設定
	void SetTranslate(const Vector3& translate) { worldTransform_.SetTranslate(translate); }

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
	uint32_t lifeFrame_ = PlayerBulletDefaults::kLifeFrames;

	// 速度（定数化）
	static inline constexpr float kSpeed_ = PlayerBulletDefaults::kSpeed;

	// シリアルナンバー
	uint32_t serialNumber_ = 0;

	// 次のシリアルナンバー
	uint32_t nextSerialNumber_ = 0;

	// 半径
	float radius_ = PlayerBulletDefaults::kRadius;
};


#pragma once
#include "Collider.h"
#include <WorldTransform.h>
#include <memory>
#include <Object3d.h>
#include <string>
#include <cstdint>

// 前方宣言
class Player;
struct Vector3;

/// <summary>
/// プレイヤー弾の調整用定数（マジックナンバー排除）
/// </summary>
namespace PlayerBulletDefaults {
	inline constexpr uint32_t kLifeFrames   = 360;   // 生存フレーム
	inline constexpr float    kSpeed        = 0.2f;  // 移動速度
	inline constexpr float    kRadius       = 0.5f;  // 当たり半径
	inline constexpr Vector3  kInitScale    = { 1.0f, 1.0f, 1.0f };
	inline constexpr Vector3  kInitRotate   = { 0.0f, 0.0f, 0.0f };
}

/// <summary>
/// プレイヤー弾のパラメータ構造体（JSONから読み込み）
/// </summary>
struct PlayerBulletParameters {
	// 生存フレーム
	uint32_t lifeFrames = PlayerBulletDefaults::kLifeFrames;
	// 移動速度
	float speed = PlayerBulletDefaults::kSpeed;
	// 当たり半径
	float radius = PlayerBulletDefaults::kRadius;
	// 初期スケール
	Vector3 initScale = PlayerBulletDefaults::kInitScale;
	// 初期回転
	Vector3 initRotate = PlayerBulletDefaults::kInitRotate;
	// 速度ベクトル（デフォルトは右方向）
	Vector3 velocityDirection = { 1.0f, 0.0f, 0.0f };
	// モデルファイル名
	std::string modelFileName = "player_bullet.obj";
};

/// <summary>
/// プレイヤーの弾
/// </summary>
class PlayerBullet : public Collider
{
public:
	/*------メンバ関数------*/

	// コンストラクタ
	PlayerBullet();

	// 初期化
	virtual void Initialize(const Vector3& position);

	// パラメータファイルから初期化
	virtual void Initialize(const Vector3& position, const std::string& parameterFileName);

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

	// パラメータの取得
	const PlayerBulletParameters& GetParameters() const { return parameters_; }

	/*------セッター------*/

	// プレイヤーの設定
	void SetPlayer(Player* player) { player_ = player; }

	// 座標の設定
	void SetTranslate(const Vector3& translate) { worldTransform_.SetTranslate(translate); }

	// パラメータの設定
	void SetParameters(const PlayerBulletParameters& parameters);

	// デフォルトパラメータを設定
	static void SetDefaultParameters(const PlayerBulletParameters& parameters);

	// デフォルトパラメータを取得
	static const PlayerBulletParameters& GetDefaultParameters();

protected:
	/*------メンバ変数------*/

	// パラメータ（JSONから読み込み）
	PlayerBulletParameters parameters_;

	// ワールド変換
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

	// シリアルナンバー
	uint32_t serialNumber_ = 0;

	// 次のシリアルナンバー
	uint32_t nextSerialNumber_ = 0;

	// 半径
	float radius_ = PlayerBulletDefaults::kRadius;

	// デフォルトパラメータ（静的メンバ）
	static inline PlayerBulletParameters defaultParameters_;
};


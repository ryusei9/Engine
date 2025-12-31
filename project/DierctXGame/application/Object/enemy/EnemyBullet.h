#pragma once
#include "Collider.h"
#include <WorldTransform.h>
#include <memory>
#include <Object3d.h>
#include <string>
#include <cstdint>

// 前方宣言
struct Vector3;

/// <summary>
/// 敵の弾の調整用定数（マジックナンバー排除）
/// </summary>
namespace EnemyBulletDefaults {
	inline constexpr uint32_t kLifeFrames = 180;   // 生存フレーム数
	inline constexpr float    kRadius     = 0.05f; // 当たり判定半径
	inline constexpr Vector3  kInitScale  = { 1.0f, 1.0f, 1.0f };
	inline constexpr Vector3  kInitRotate = { 0.0f, 0.0f, 0.0f };
}

/// <summary>
/// 敵の弾のパラメータ構造体（JSONから読み込み）
/// </summary>
struct EnemyBulletParameters {
	// 生存フレーム数
	uint32_t lifeFrames = EnemyBulletDefaults::kLifeFrames;
	// 当たり判定半径
	float radius = EnemyBulletDefaults::kRadius;
	// 初期スケール
	Vector3 initScale = EnemyBulletDefaults::kInitScale;
	// 初期回転
	Vector3 initRotate = EnemyBulletDefaults::kInitRotate;
	// モデルファイル名
	std::string modelFileName = "player_bullet.obj";
};

/// <summary>
/// 敵の弾クラス
/// </summary>
class EnemyBullet : public Collider
{
public:
	// 初期化
	void Initialize(const Vector3& position, const Vector3& velocity);

	// パラメータファイルから初期化
	void Initialize(const Vector3& position, const Vector3& velocity, const std::string& parameterFileName);

	// 更新
	void Update();
    
	// 描画
	void Draw();

	// 移動
	void Move();

	// 衝突判定
	void OnCollision(Collider* other) override;

	// 中心座標を取得
	Vector3 GetCenterPosition() const override;

	// 生存状態を取得
	bool IsAlive() const { return isAlive_; }

	// 位置を取得（コピー回避）
	const Vector3& GetPosition() const { return worldTransform_.GetTranslate(); }

	// 位置を設定
	void SetPosition(const Vector3& position) { worldTransform_.SetTranslate(position); }

	// パラメータ設定
	void SetParameters(const EnemyBulletParameters& parameters);

	// パラメータ取得
	const EnemyBulletParameters& GetParameters() const { return parameters_; }

	// デフォルトパラメータを設定
	static void SetDefaultParameters(const EnemyBulletParameters& parameters);

	// デフォルトパラメータを取得
	static const EnemyBulletParameters& GetDefaultParameters();

private:
	// パラメータ
	EnemyBulletParameters parameters_;

	// ワールド変形情報
	WorldTransform worldTransform_;

	// 弾のオブジェクト
	std::unique_ptr<Object3d> objectBullet_;

	// 弾の速度
	Vector3 velocity_ = {};

	// 生存状態
	bool isAlive_ = true;

	// 生存フレーム数
	uint32_t lifeFrame_ = EnemyBulletDefaults::kLifeFrames;

	// 半径
	float radius_ = EnemyBulletDefaults::kRadius;

	// デフォルトパラメータ（静的メンバ）
	static inline EnemyBulletParameters defaultParameters_;
};


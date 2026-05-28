#pragma once
#include <string>
#include <cstdint>
#include "LineCollider.h"

// 前方宣言
struct Vector3;
class Player;
using namespace MyEngine;
/// <summary>
/// チャージ弾の調整用定数（マジックナンバー排除）
/// </summary>
namespace PlayerChargeBulletDefaults {
	inline constexpr float    kDamage        = 30.0f;
	inline constexpr float    kRadius        = 0.5f;
	inline constexpr float    kScaleFactor   = 10.0f;
	inline constexpr uint32_t kSerialStart   = 1u;
}

/// <summary>
/// プレイヤーチャージ弾のパラメータ構造体（JSONから読み込み）
/// </summary>
struct PlayerChargeBulletParameters {
	// ダメージ数
	float damage = PlayerChargeBulletDefaults::kDamage;
	// 当たり半径
	float radius = PlayerChargeBulletDefaults::kRadius;
	// 見た目スケール倍率
	float scaleFactor = PlayerChargeBulletDefaults::kScaleFactor;
	// シリアルナンバー開始値
	uint32_t serialStart = PlayerChargeBulletDefaults::kSerialStart;
	
	// 基底クラスのパラメータ（継承して上書き可能）
	//PlayerBulletParameters baseBulletParams;
};

/// <summary>
/// プレイヤーのチャージ弾クラス
/// </summary>
class PlayerChargeBullet : public LineCollider {
public:
	// コンストラクタ
	PlayerChargeBullet();

	// 初期化
	void Initialize(Player* player);

	// パラメータファイルから初期化
	void Initialize(Player* player, const std::string& parameterFileName);

	// 更新
	void Update();

	// 描画
	void Draw();

	// 衝突判定（敵を貫通する）
	void OnCollision(Collider* other) override;

	// ダメージ数の取得
	float GetDamage() const { return damage_; }

	// シリアルナンバーの取得
	uint32_t GetSerialNumber() const { return serialNumber_; }

	bool IsAlive() const { return isAlive_; }

	// パラメータの取得
	const PlayerChargeBulletParameters& GetChargeBulletParameters() const { return chargeBulletParameters_; }

	// パラメータの設定
	void SetChargeBulletParameters(const PlayerChargeBulletParameters& parameters);

	// デフォルトパラメータを設定
	static void SetDefaultChargeBulletParameters(const PlayerChargeBulletParameters& parameters);

	// デフォルトパラメータを取得
	static const PlayerChargeBulletParameters& GetDefaultChargeBulletParameters();

private:
	/*------メンバ変数------*/

	Player* player_ = nullptr;

	float length_ = 30.0f;

	float duration_ = 2.0f;

	float timer_ = 0.0f;

	bool isAlive_ = true;
	// チャージ弾専用パラメータ
	PlayerChargeBulletParameters chargeBulletParameters_;

	// ダメージ数
	float damage_ = PlayerChargeBulletDefaults::kDamage;

	// シリアルナンバー
	uint32_t serialNumber_ = 0;

	// 次のシリアルナンバー
	static uint32_t sNextSerialNumber_;

	// デフォルトパラメータ（静的メンバ）
	static inline PlayerChargeBulletParameters defaultChargeBulletParameters_;

	/*Vector3 start_;
	Vector3 end_;
	float radius_;*/
};
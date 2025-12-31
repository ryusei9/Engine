#pragma once
#include "PlayerBullet.h"
#include "Collider.h"
#include <string>
#include <cstdint>

// 前方宣言
struct Vector3;

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
	PlayerBulletParameters baseBulletParams;
};

/// <summary>
/// プレイヤーのチャージ弾クラス
/// </summary>
class PlayerChargeBullet : public PlayerBullet {
public:
	// コンストラクタ
	PlayerChargeBullet();

	// 初期化
	void Initialize(const Vector3& position) override;

	// パラメータファイルから初期化
	void Initialize(const Vector3& position, const std::string& parameterFileName);

	// 更新
	void Update() override;

	// 描画
	void Draw() override;

	// ダメージ数の取得
	float GetDamage() const { return damage_; }

	// シリアルナンバーの取得
	uint32_t GetSerialNumber() const { return serialNumber_; }

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
};
#pragma once
#include "PlayerBullet.h"
#include "Collider.h"

/// <summary>
/// チャージ弾の調整用定数（マジックナンバー排除）
/// </summary>
namespace PlayerChargeBulletDefaults {
	inline constexpr float    kDamage        = 30.0f;
	inline constexpr float    kRadius        = 0.5f;      // 通常弾より大きめ
	inline constexpr float    kScaleFactor   = 10.0f;     // 見た目スケール倍率
	inline constexpr uint32_t kSerialStart   = 1u;
}

/// <summary>
/// プレイヤーのチャージ弾クラス
/// </summary>
class PlayerChargeBullet : public PlayerBullet {
public:
	// コンストラクタ
	PlayerChargeBullet();

	// 初期化
	void Initialize(const Vector3& position) override;

	// 更新
	void Update() override;

	// 描画
	void Draw() override;

	// ダメージ数の取得
	float GetDamage() const { return damage_; }

	// シリアルナンバーの取得
	uint32_t GetSerialNumber() const { return serialNumber_; }

private:
	/*------メンバ変数------*/

	// ダメージ数
	float damage_ = PlayerChargeBulletDefaults::kDamage;

	// シリアルナンバー
	uint32_t serialNumber_ = 0;

	// 次のシリアルナンバー
	static uint32_t sNextSerialNumber_;
};
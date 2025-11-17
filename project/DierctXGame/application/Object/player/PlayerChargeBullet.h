#pragma once
#include "PlayerBullet.h"
#include "Collider.h"

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
    float damage_ = 30.0f;

	// シリアルナンバー
    uint32_t serialNumber_ = 0;

	// 次のシリアルナンバー
    static uint32_t nextSerialNumber_;
};
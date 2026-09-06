#pragma once
#include <string>
#include <cstdint>
#include "LineCollider.h"
#include "ParticleEmitter.h"
#include <memory>
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
	/// <summary>
	/// ダメージ数
	/// </summary>
	float damage = PlayerChargeBulletDefaults::kDamage;

	/// <summary>
	/// 当たり半径
	/// </summary>
	float radius = PlayerChargeBulletDefaults::kRadius;

	/// <summary>
	/// 見た目スケール倍率
	/// </summary>
	float scaleFactor = PlayerChargeBulletDefaults::kScaleFactor;

	/// <summary>
	/// シリアルナンバー開始値
	/// </summary>
	uint32_t serialStart = PlayerChargeBulletDefaults::kSerialStart;
};

/// <summary>
/// プレイヤーのチャージ弾クラス
/// </summary>
class PlayerChargeBullet : public LineCollider {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	PlayerChargeBullet();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="player">プレイヤーパーティへのポインタ</param>
	void Initialize(Player* player);

	/// <summary>
	/// パラメータファイルから初期化
	/// </summary>
	/// <param name="player">プレイヤー情報</param>
	/// <param name="parameterFileName">JSONファイル名</param>
	void Initialize(Player* player, const std::string& parameterFileName);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 衝突判定（敵を貫通する）
	/// </summary>
	/// <param name="other">衝突した他のコライダー</param>
	void OnCollision(Collider* other) override;

	/// <summary>
	/// デバッグ用レーザーパーティクルの描画
	/// </summary>
	/// <param name="start">開始位置</param>
	/// <param name="end">終了位置</param>
	void DebugLaserParticle(const Vector3& start, const Vector3& end);

	/// <summary>
	/// ImGuiによるデバッグ情報描画
	/// </summary>
	void DrawImGui();

	/// <summary>
	/// ダメージ数の取得
	/// </summary>
	float GetDamage() const { return damage_; }

	/// <summary>
	/// シリアルナンバーの取得
	/// </summary>
	uint32_t GetSerialNumber() const { return serialNumber_; }

	/// <summary>
	/// 生存フラグの取得
	/// </summary>
	bool IsAlive() const { return isAlive_; }

	/// <summary>
	/// パラメータの取得
	/// </summary>
	const PlayerChargeBulletParameters& GetChargeBulletParameters() const { return chargeBulletParameters_; }

	/// <summary>
	/// パラメータの設定
	/// </summary>
	void SetChargeBulletParameters(const PlayerChargeBulletParameters& parameters);

	/// <summary>
	/// デフォルトパラメータを設定
	/// </summary>
	static void SetDefaultChargeBulletParameters(const PlayerChargeBulletParameters& parameters);

	/// <summary>
	/// デフォルトパラメータを取得
	/// </summary>
	static const PlayerChargeBulletParameters& GetDefaultChargeBulletParameters();

private:
	/*------メンバ変数------*/

	Player* player_ = nullptr;

	Vector3 offset_ = { 0.0f, 0.0f, 0.0f };

	float length_ = 13.0f;

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

	std::unique_ptr<ParticleEmitter> debugParticleEmitter_;
};
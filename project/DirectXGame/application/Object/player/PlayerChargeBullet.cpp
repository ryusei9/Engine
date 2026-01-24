#include "PlayerChargeBullet.h"
#include <CollisionTypeIdDef.h>
#include "JsonLoader.h"

// 静的メンバ変数の初期化
uint32_t PlayerChargeBullet::sNextSerialNumber_ = PlayerChargeBulletDefaults::kSerialStart;

PlayerChargeBullet::PlayerChargeBullet()
{
	// シリアルナンバーを設定
	serialNumber_ = sNextSerialNumber_++;
}

void PlayerChargeBullet::Initialize(const Vector3& position)
{
	// デフォルトパラメータで初期化
	Initialize(position, "");
}

void PlayerChargeBullet::Initialize(const Vector3& position, const std::string& parameterFileName)
{
	// パラメータファイルから読み込み（空文字列の場合はデフォルトパラメータを使用）
	if (!parameterFileName.empty()) {
		chargeBulletParameters_ = JsonLoader::LoadPlayerChargeBulletParameters(parameterFileName);
	} else {
		chargeBulletParameters_ = defaultChargeBulletParameters_;
	}

	// パラメータを適用
	damage_ = chargeBulletParameters_.damage;

	// 基底クラスのパラメータを設定してから初期化
	PlayerBullet::SetParameters(chargeBulletParameters_.baseBulletParams);
	
	// 基底の初期化（位置・Transformなど）
	// 空文字列を渡すことで、既に設定されたパラメータを使用
	PlayerBullet::Initialize(position, "");

	// チャージ弾のコライダーID（基底クラスの初期化後に上書き）
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kPlayerChargeBullet));

	// worldTransformのスケールを拡大（パラメータから倍率取得）
	const Vector3 scaledTransform = worldTransform_.GetScale() * chargeBulletParameters_.scaleFactor;
	worldTransform_.SetScale(scaledTransform);

	// 見た目のスケールも拡大
	if (objectBullet_) {
		objectBullet_->SetScale(scaledTransform);
	}

	// 当たり判定の半径も拡大（パラメータから取得）
	SetRadius(chargeBulletParameters_.radius);
	radius_ = chargeBulletParameters_.radius;
}

void PlayerChargeBullet::Update()
{
	// 共通の更新（移動・寿命・描画更新）
	PlayerBullet::Update();
	// 必要ならチャージ弾専用の挙動をここに追加
}

void PlayerChargeBullet::Draw()
{
	// チャージ弾専用の描画があれば追加
	PlayerBullet::Draw();
}

void PlayerChargeBullet::SetChargeBulletParameters(const PlayerChargeBulletParameters& parameters)
{
	chargeBulletParameters_ = parameters;
	damage_ = chargeBulletParameters_.damage;
	SetRadius(chargeBulletParameters_.radius);
	radius_ = chargeBulletParameters_.radius;
	
	// 基底クラスのパラメータも設定
	PlayerBullet::SetParameters(chargeBulletParameters_.baseBulletParams);
}

void PlayerChargeBullet::SetDefaultChargeBulletParameters(const PlayerChargeBulletParameters& parameters)
{
	defaultChargeBulletParameters_ = parameters;
	// 基底クラスのデフォルトパラメータも設定
	PlayerBullet::SetDefaultParameters(parameters.baseBulletParams);
}

const PlayerChargeBulletParameters& PlayerChargeBullet::GetDefaultChargeBulletParameters()
{
	return defaultChargeBulletParameters_;
}
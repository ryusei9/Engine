#include "PlayerChargeBullet.h"
#include <CollisionTypeIdDef.h>
#include "JsonLoader.h"
#include <Player.h>
#include <WorldTransform.h>

// 静的メンバ変数の初期化
uint32_t PlayerChargeBullet::sNextSerialNumber_ = PlayerChargeBulletDefaults::kSerialStart;

PlayerChargeBullet::PlayerChargeBullet()
{
	// シリアルナンバーを設定
	serialNumber_ = sNextSerialNumber_++;
}

void PlayerChargeBullet::Initialize(Player* player)
{
	// デフォルトパラメータで初期化
	Initialize(player, "");
}

void PlayerChargeBullet::Initialize(Player* player, const std::string& parameterFileName)
{
	if (!player) {
		return;
	}
	// パラメータファイルから読み込み（空文字列の場合はデフォルトパラメータを使用）
	if (!parameterFileName.empty()) {
		chargeBulletParameters_ = JsonLoader::LoadPlayerChargeBulletParameters(parameterFileName);
	} else {
		chargeBulletParameters_ = defaultChargeBulletParameters_;
	}
	player_ = player;

	/*worldTransform_.Initialize();*/

	// パラメータを適用
	damage_ = chargeBulletParameters_.damage;

	// 基底クラスのパラメータを設定してから初期化
	//PlayerBullet::SetParameters(chargeBulletParameters_.baseBulletParams);
	
	// 基底の初期化（位置・Transformなど）
	// 空文字列を渡すことで、既に設定されたパラメータを使用
	//PlayerBullet::Initialize(position, "");

	// チャージ弾のコライダーID（基底クラスの初期化後に上書き）
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kPlayerChargeBullet));

	//// worldTransformのスケールを拡大（パラメータから倍率取得）
	//const Vector3 scaledTransform = worldTransform_.GetScale() * chargeBulletParameters_.scaleFactor;
	//worldTransform_.SetScale(scaledTransform);

	// 見た目のスケールも拡大
	/*if (objectBullet_) {
		objectBullet_->SetScale(scaledTransform);
	}*/

	// 当たり判定の半径も拡大（パラメータから取得）
	SetRadius(chargeBulletParameters_.radius);
	//chargeBulletParameters_.radius = chargeBulletParameters_.radius;
}

void PlayerChargeBullet::Update()
{
	// 必要ならチャージ弾専用の挙動をここに追加
	if (!player_) {
		return;
	}

	Vector3 playerPos = player_->GetPosition();

	Vector3 dir = { 1.0f, 0.0f, 0.0f };

	Vector3 end = playerPos + dir * length_;

	SetStart(playerPos);
	SetEnd(end);

	//SetRadius(1.0f);

	timer_ += 1.0f / 60.0f;

	if (timer_ >= duration_) {
		isAlive_ = false;
	}
	
}

void PlayerChargeBullet::Draw()
{
	// チャージ弾専用の描画があれば追加
	//PlayerBullet::Draw();
}

void PlayerChargeBullet::OnCollision(Collider* other)
{
	// チャージ弾は敵を貫通するため、何もしない
	// 寿命で消えるのみ
	(void)other; // 未使用パラメータ警告回避
}

void PlayerChargeBullet::SetChargeBulletParameters(const PlayerChargeBulletParameters& parameters)
{
	chargeBulletParameters_ = parameters;
	damage_ = chargeBulletParameters_.damage;
	SetRadius(chargeBulletParameters_.radius);
	
	// 基底クラスのパラメータも設定
	//PlayerBullet::SetParameters(chargeBulletParameters_.baseBulletParams);
}

void PlayerChargeBullet::SetDefaultChargeBulletParameters(const PlayerChargeBulletParameters& parameters)
{
	defaultChargeBulletParameters_ = parameters;
	// 基底クラスのデフォルトパラメータも設定
	//PlayerBullet::SetDefaultParameters(parameters.baseBulletParams);
}

const PlayerChargeBulletParameters& PlayerChargeBullet::GetDefaultChargeBulletParameters()
{
	return defaultChargeBulletParameters_;
}
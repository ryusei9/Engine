#include "PlayerBullet.h"
#include <CollisionTypeIdDef.h>
#include "JsonLoader.h"

PlayerBullet::PlayerBullet() = default;

void PlayerBullet::Initialize(const Vector3& position)
{
	// デフォルトパラメータで初期化
	Initialize(position, "");
}

void PlayerBullet::Initialize(const Vector3& position, const std::string& parameterFileName)
{
	// パラメータファイルから読み込み（空文字列の場合はデフォルトパラメータを使用）
	if (!parameterFileName.empty()) {
		parameters_ = JsonLoader::LoadPlayerBulletParameters(parameterFileName);
	} else {
		parameters_ = defaultParameters_;
	}

	// ColliderにIDをセット
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kPlayerBullet));

	// 初期化
	isAlive_   = true;
	lifeFrame_ = parameters_.lifeFrames;
	radius_    = parameters_.radius;

	worldTransform_.Initialize();
	worldTransform_.SetScale(parameters_.initScale);
	worldTransform_.SetRotate(parameters_.initRotate);
	worldTransform_.SetTranslate(position);

	// 弾オブジェクト
	objectBullet_ = std::make_unique<Object3d>();
	objectBullet_->Initialize(parameters_.modelFileName);
	objectBullet_->SetScale(worldTransform_.GetScale());
	objectBullet_->SetTranslate(worldTransform_.GetTranslate());

	// 初速を設定（パラメータから取得）
	velocity_ = parameters_.velocityDirection * parameters_.speed;

	// 半径を設定
	SetRadius(parameters_.radius);
}

void PlayerBullet::Update()
{
	// 移動
	Move();

	// 寿命
	if (lifeFrame_ > 0) {
		--lifeFrame_;
	} else {
		isAlive_ = false;
	}

	// 変換更新と反映
	worldTransform_.Update();
	objectBullet_->SetScale(worldTransform_.GetScale());
	objectBullet_->SetTranslate(worldTransform_.GetTranslate());
	objectBullet_->Update();
}

void PlayerBullet::Draw()
{
	if (!isAlive_) {
		return;
	}
	objectBullet_->Draw();
}

void PlayerBullet::Move()
{
	// 速度に基づいて位置更新
	worldTransform_.SetTranslate(worldTransform_.GetTranslate() + velocity_);
}

void PlayerBullet::OnCollision(Collider* other)
{
	if (!isAlive_) {
		return;
	}

	// 敵や敵弾に当たったら消える（必要に応じて条件拡張）
	const uint32_t type = other->GetTypeID();
	if (type == static_cast<uint32_t>(CollisionTypeIdDef::kEnemy) ||
		type == static_cast<uint32_t>(CollisionTypeIdDef::kEnemyBullet)) {
		isAlive_ = false;
		SetRadius(0.0f); // 直ちに当たり判定無効化
	}
}

Vector3 PlayerBullet::GetCenterPosition() const
{
	return worldTransform_.GetTranslate();
}

void PlayerBullet::SetParameters(const PlayerBulletParameters& parameters)
{
	parameters_ = parameters;
	lifeFrame_ = parameters_.lifeFrames;
	radius_ = parameters_.radius;
	SetRadius(parameters_.radius);
	velocity_ = parameters_.velocityDirection * parameters_.speed;
}

void PlayerBullet::SetDefaultParameters(const PlayerBulletParameters& parameters)
{
	defaultParameters_ = parameters;
}

const PlayerBulletParameters& PlayerBullet::GetDefaultParameters()
{
	return defaultParameters_;
}

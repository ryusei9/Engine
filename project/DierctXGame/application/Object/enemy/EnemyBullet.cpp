#include "EnemyBullet.h"
#include <CollisionTypeIdDef.h>
#include "JsonLoader.h"

void EnemyBullet::Initialize(const Vector3& position, const Vector3& velocity)
{
	// デフォルトパラメータで初期化
	Initialize(position, velocity, "");
}

void EnemyBullet::Initialize(const Vector3& position, const Vector3& velocity, const std::string& parameterFileName)
{
	// パラメータファイルから読み込み（空文字列の場合はデフォルトパラメータを使用）
	if (!parameterFileName.empty()) {
		parameters_ = JsonLoader::LoadEnemyBulletParameters(parameterFileName);
	} else {
		parameters_ = defaultParameters_;
	}

	// ColliderにIDをセット
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kEnemyBullet));

	// 初期化処理
	isAlive_   = true;
	lifeFrame_ = parameters_.lifeFrames;
	radius_    = parameters_.radius;

	worldTransform_.Initialize();
	worldTransform_.SetScale(parameters_.initScale);
	worldTransform_.SetRotate(parameters_.initRotate);
	worldTransform_.SetTranslate(position);

	velocity_ = velocity;

	// オブジェクトを設定
	objectBullet_ = std::make_unique<Object3d>();
	objectBullet_->Initialize(parameters_.modelFileName);
	objectBullet_->SetScale(worldTransform_.GetScale());
	objectBullet_->SetTranslate(worldTransform_.GetTranslate());

	// 半径を設定
	SetRadius(parameters_.radius);
}

void EnemyBullet::Update()
{
	// 移動処理
	Move();

	// 生存フレームのカウントダウン
	if (lifeFrame_ > 0) {
		--lifeFrame_;
	} else {
		isAlive_ = false;
	}

	// 変換更新と描画パラメータ反映
	worldTransform_.Update();
	objectBullet_->SetScale(worldTransform_.GetScale());
	objectBullet_->SetTranslate(worldTransform_.GetTranslate());
	objectBullet_->Update();
}

void EnemyBullet::Draw()
{
	// 描画処理
	objectBullet_->Draw();
}

void EnemyBullet::Move()
{
	// 速度ベクトルに基づいて位置を更新
	worldTransform_.SetTranslate(worldTransform_.GetTranslate() + velocity_);
}

void EnemyBullet::OnCollision(Collider* other)
{
	if (!isAlive_) {
		return;
	}

	// プレイヤーと衝突した場合に消滅
	if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeIdDef::kPlayer)) {
		isAlive_ = false;
		// 半径を0にして当たり判定も即無効化
		SetRadius(0.0f);
	}
}

Vector3 EnemyBullet::GetCenterPosition() const
{
	return worldTransform_.GetTranslate();
}

void EnemyBullet::SetParameters(const EnemyBulletParameters& parameters)
{
	parameters_ = parameters;
	lifeFrame_ = parameters_.lifeFrames;
	radius_ = parameters_.radius;
	SetRadius(parameters_.radius);
}

void EnemyBullet::SetDefaultParameters(const EnemyBulletParameters& parameters)
{
	defaultParameters_ = parameters;
}

const EnemyBulletParameters& EnemyBullet::GetDefaultParameters()
{
	return defaultParameters_;
}
#include "EnemyBullet.h"
#include <CollisionTypeIdDef.h>

void EnemyBullet::Initialize(const Vector3& position, const Vector3& velocity)
{
	// ColliderにIDをセット
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kEnemyBullet));

	// 初期化処理
	isAlive_   = true;
	lifeFrame_ = EnemyBulletDefaults::kLifeFrames;

	worldTransform_.Initialize();
	worldTransform_.SetScale(EnemyBulletDefaults::kInitScale);
	worldTransform_.SetRotate(EnemyBulletDefaults::kInitRotate);
	worldTransform_.SetTranslate(position);

	velocity_ = velocity;

	// オブジェクトを設定
	objectBullet_ = std::make_unique<Object3d>();
	objectBullet_->Initialize("player_bullet.obj");
	objectBullet_->SetScale(worldTransform_.GetScale());
	objectBullet_->SetTranslate(worldTransform_.GetTranslate());

	// 半径を設定
	SetRadius(EnemyBulletDefaults::kRadius);
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
	if (!isAlive_) return;

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
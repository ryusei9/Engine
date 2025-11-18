#include "EnemyBullet.h"
#include <CollisionTypeIdDef.h>

void EnemyBullet::Initialize(const Vector3& position, const Vector3& velocity)
{
    // ColliderにIDをセット
    Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kEnemyBullet));

	// 初期化処理
    isAlive_ = true;
    lifeFrame_ = 180;
    worldTransform_.Initialize();
    worldTransform_.scale_ = { 1.0f, 1.0f, 1.0f };
    worldTransform_.rotate_ = { 0.0f, 0.0f, 0.0f };
    worldTransform_.translate_ = position;
    velocity_ = velocity;

    // オブジェクトを設定
    objectBullet_ = std::make_unique<Object3d>();
    objectBullet_->Initialize("player_bullet.obj");
    objectBullet_->SetScale(worldTransform_.scale_);
    objectBullet_->SetTranslate(worldTransform_.translate_);

	// 半径を設定
    SetRadius(radius_);
}

void EnemyBullet::Update()
{
	// 移動処理
    Move();

	// 生存フレームのカウントダウン
    if (lifeFrame_ > 0) lifeFrame_--;
	// 生存フレームが0になったら消滅
    else isAlive_ = false;
	// 更新処理
    worldTransform_.Update();
    objectBullet_->SetScale(worldTransform_.scale_);
    objectBullet_->SetTranslate(worldTransform_.translate_);
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
    worldTransform_.translate_ += velocity_;
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
    return worldTransform_.translate_;
}
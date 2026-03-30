#include "PlayerLaser.h"
#include <Normalize.h>

void PlayerLaser::Initialize(const Vector3& start, const Vector3& dir)
{
    startPos_ = start;
    direction_ = Normalize(dir);
    length_ = 10.0f;
    isActive_ = true;

	velocity_ = direction_ * speed_; // 右方向に速度を設定
	worldTransform_.Initialize();
	worldTransform_.SetTranslate(startPos_);
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize("player_bullet.obj");
}

void PlayerLaser::Update()
{
    if (!isActive_) return;

    // 寿命
    if (lifeFrame_ > 0) {
        --lifeFrame_;
    }
    else {
        isActive_ = false;
    }
    

    worldTransform_.SetTranslate(worldTransform_.GetTranslate() + velocity_);

    // 長さ方向にスケール
    worldTransform_.SetScale({ length_, 0.1f, 0.1f });

    worldTransform_.Update();

    object3d_->SetTranslate(worldTransform_.GetTranslate());
    object3d_->SetScale(worldTransform_.GetScale());
    object3d_->Update();
}

void PlayerLaser::Draw()
{
    if (isActive_) {
        object3d_->Draw();
	}
}

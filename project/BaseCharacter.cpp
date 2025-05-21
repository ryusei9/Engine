#include "BaseCharacter.h"
#include <Object3DCommon.h>
void BaseCharacter::Initialize()
{

	input_ = Input::GetInstance();
	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();

	// ワールド変換の初期化
	worldTransform_.scale = { 1.0f,1.0f,1.0f };
	worldTransform_.rotate = { 0.0f,0.0f,0.0f };
	worldTransform_.translate = { 0.0f,0.0f,0.0f };
}

void BaseCharacter::Update()
{
	// ワールド変換の更新
	object3d_->SetCamera(camera_);
	object3d_->SetTranslate(worldTransform_.translate);
	object3d_->SetRotate(worldTransform_.rotate);
	object3d_->SetScale(worldTransform_.scale);
	object3d_->Update();
}

void BaseCharacter::Draw()
{
	// 3Dオブジェクトの描画
	object3d_->Draw();
}

void BaseCharacter::OnCollision(Collider* other)
{

}

Vector3 BaseCharacter::GetCenterPosition() const
{
	return Vector3();
}

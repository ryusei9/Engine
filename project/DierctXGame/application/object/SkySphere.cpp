#include "SkySphere.h"

void SkySphere::Initialize(Object3d* model)
{
	assert(model);
	model_ = model;
	transform_ = { model_->GetScale(),model_->GetRotate(),model_->GetTranslate()};
}

void SkySphere::Update()
{
	model_->SetTranslate(transform_.translate);
	model_->SetScale(transform_.scale);
	model_->SetRotate(transform_.rotate);
	model_->Update();
}

void SkySphere::Draw()
{
	model_->Draw();
}

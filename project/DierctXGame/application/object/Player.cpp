#include "Player.h"

void Player::Initialize(Object3d* model)
{
	// 入力の初期化
	input_ = Input::GetInstance();
	model_ = model;
}

void Player::Update()
{
	// 入力の更新
	input_->Update();
	// 操作
	if (input_->PushKey(DIK_RIGHTARROW)) {
		worldTranform.translate.x -= 0.1f;

	}
	if (input_->PushKey(DIK_LEFTARROW)) {
		worldTranform.translate.x += 0.1f;
	}
}

void Player::Draw()
{
	// 描画処理
}

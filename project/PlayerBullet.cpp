#include "PlayerBullet.h"
#include <Vector3.h> // Add this include to resolve the Add function
#include <Add.h>

PlayerBullet::~PlayerBullet()
{
}

void PlayerBullet::Initialize(Object3d* model, const Vector3& position, const Vector3& velocity)
{
	// NULLポインタチェック
	assert(model);
	// モデルをセット
	//model_ = model;
	
	model_ = model;
	// テクスチャ読み込み
	//textureHandle_ = TextureManager::GetInstance()->Load("white.png");
	// ワールドトランスフォームの初期化
	transform_ = {
		{0.5f,0.5f,0.5f},
		{0.0f,0.0f,0.0f},
		position
	};

	
	// 引数で受け取った速度をメンバ変数に代入
	velocity_ = velocity;
	
}

void PlayerBullet::Update()
{
	// 弾の移動処理
	transform_.translate = Add(transform_.translate,velocity_);

	// 寿命タイマーを減らす
	--deathTimer_;
	if (deathTimer_ <= 0) {
		isDead_ = true;
	}

	model_->SetTranslate(transform_.translate);
	model_->SetScale(transform_.scale);
	model_->SetRotate(transform_.rotate);

	// モデルの更新
	model_->Update();
}

void PlayerBullet::Draw()
{
	model_->Draw();
}

void PlayerBullet::OnCollision()
{
	isDead_ = true;
}

Vector3 PlayerBullet::GetWorldPosition()
{
	return transform_.translate;
}

#include "PlayerBullet.h"
#include <Vector3.h> // Add this include to resolve the Add function
#include <Add.h>
#include <ImGui.h>

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
	worldTransform_.Initialize();
	worldTransform_.scale_ = { 0.5f,0.1f,0.1f };
	worldTransform_.translate_ = position;
	isDead_ = false;
	deathTimer_ = kLifeTime;
	// 引数で受け取った速度をメンバ変数に代入
	velocity_ = velocity;
	
}

void PlayerBullet::Update()
{
	// 弾の移動処理
	worldTransform_.translate_ = Add(worldTransform_.translate_,velocity_);
	worldTransform_.UpdateMatrix();
	//transform_.translate = {0.0f,2.0f,0.0f};
	// 寿命タイマーを減らす
	--deathTimer_;
	if (deathTimer_ <= 0) {
		isDead_ = true;
	}
	model_->SetScale(worldTransform_.scale_);
	model_->SetRotate(worldTransform_.rotate_);
	model_->SetTranslate(worldTransform_.translate_);
	

	// モデルの更新
	model_->Update();
}

void PlayerBullet::Draw()
{
	model_->Draw();
}

void PlayerBullet::OnCollision()
{
	//transform_.translate = { 0.0f,-1000.0f,0.0f };
	isDead_ = true;
}

Vector3 PlayerBullet::GetWorldPosition()
{
	// ワールド座標を入れる変数
	Vector3 worldPos = {};
	// ワールド行列の平行移動成分を取得(ワールド座標)
	worldPos.x = worldTransform_.translate_.x;
	worldPos.y = worldTransform_.translate_.y;
	worldPos.z = worldTransform_.translate_.z;

	return worldPos;
}

void PlayerBullet::DrawImGui()
{
	ImGui::Text("Bullet");
	ImGui::SliderFloat3("Position", &worldTransform_.translate_.x, -10.0f, 10.0f);
	ImGui::SliderFloat3("Scale", &worldTransform_.scale_.x, 0.0f, 10.0f);
	ImGui::SliderFloat3("Rotate", &worldTransform_.rotate_.x, -180.0f, 180.0f);
	ImGui::Text("Velocity");
	ImGui::SliderFloat3("Velocity", &velocity_.x, -1.0f, 1.0f);
	ImGui::Text("LifeTime");
	ImGui::SliderInt("LifeTime", &deathTimer_, 0, kLifeTime);
	ImGui::Checkbox("IsDead", &isDead_);
	ImGui::Separator();
}

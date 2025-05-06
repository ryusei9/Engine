#include "Player.h"
#include <Object3dCommon.h>
#include <CollisionTypeIdDef.h>
Player::Player()
{
	// シリアルナンバーを振る
	serialNumber_ = nextSerialNumber_;
	// 次のシリアルナンバーに1を足す
	++nextSerialNumber_;
}
void Player::Initialize()
{
	// プレイヤーの初期化
	BaseCharacter::Initialize();

	// プレイヤーのコライダーの設定
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kPlayer));
	// プレイヤーのワールド変換を初期化
	worldTransform_.scale = { 1.0f,1.0f,1.0f };
	worldTransform_.rotate = { 0.0f,0.0f,0.0f };
	worldTransform_.translate = { 0.0f,0.0f,0.0f };

	// プレイヤーのカメラを取得
	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();

	object3d_ = std::make_unique<Object3d>();

	// プレイヤーの3Dオブジェクトを初期化
	object3d_->Initialize("monsterBall.obj");
}

void Player::Update()
{// プレイヤーの更新
	BaseCharacter::Update();

	// プレイヤーの移動
	Move();
	// プレイヤーのワールド変換を更新
	object3d_->SetCamera(camera_);
	object3d_->SetTranslate(worldTransform_.translate);
	object3d_->SetRotate(worldTransform_.rotate);
	object3d_->SetScale(worldTransform_.scale);
	object3d_->Update();
}

void Player::Draw()
{
	BaseCharacter::Draw();
}

void Player::Move()
{
	// プレイヤーの移動
	if (input_->PushKey(DIK_W)) {
		worldTransform_.translate.y += moveSpeed_;
	}
	if (input_->PushKey(DIK_S)) {
		worldTransform_.translate.y -= moveSpeed_;
	}
	if (input_->PushKey(DIK_A)) {
		worldTransform_.translate.x -= moveSpeed_;
	}
	if (input_->PushKey(DIK_D)) {
		worldTransform_.translate.x += moveSpeed_;
	}
}

void Player::Attack()
{// プレイヤーの攻撃
	if (input_->TriggerKey(DIK_SPACE)) {
		// 攻撃処理
	}
}

void Player::OnCollision(Collider* other)
{// プレイヤーの衝突判定
	if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeIdDef::kEnemy)) {
		// 敵と衝突した場合
		hp_ -= 1;
	}
}

Vector3 Player::GetCenterPosition() const
{
	return Vector3();
}

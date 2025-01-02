#include "Player.h"

Player::~Player()
{
	for (PlayerBullet* bullet : bullets_) {
		delete bullet;
	}
	delete bulletModel_;
}

void Player::Initialize(Object3d* model, Object3d* bulletModel)
{
	// 入力の初期化
	input_ = Input::GetInstance();
	model_ = model;

	// 弾のモデルを設定
	bulletModel_ = bulletModel;

	transform.translate = model_->GetTranslate();
	transform.scale = model_->GetScale();
	transform.rotate = model_->GetRotate();
	transform.translate = { -3.0f,0.0f,0.0f };
	transform.scale = { 0.2f,0.2f,0.2f };
	transform.rotate = { 0.0f,-90.0f,0.0f };
}

void Player::Update()
{
	// デスフラグの立った弾を削除
	bullets_.remove_if([](PlayerBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
		});

	// 弾丸の更新
	for (PlayerBullet* bullet : bullets_) {

		bullet->Update();
	}
	Attack();
	// 入力の更新
	input_->Update();

	// 弾の発射位置を設定
	bulletEmitter.translate = transform.translate;
	// 操作
	if (input_->PushKey(DIK_RIGHTARROW)) {
		transform.translate.x += 0.05f;

	}
	if (input_->PushKey(DIK_LEFTARROW)) {
		transform.translate.x -= 0.05f;
	}
	if (input_->PushKey(DIK_DOWNARROW)) {
		transform.translate.y -= 0.05f;

	}
	if (input_->PushKey(DIK_UPARROW)) {
		transform.translate.y += 0.05f;
	}
	model_->SetTranslate(transform.translate);
	model_->SetScale(transform.scale);
	model_->SetRotate(transform.rotate);

	// モデルの更新
	model_->Update();
}

void Player::Draw()
{
	// 描画処理
	model_->Draw();
	// 弾描画
	for (PlayerBullet* bullet : bullets_) {
		bullet->Draw();
	}
}

void Player::Attack()
{
	fireCoolTime++;
	if (input_->GetInstance()->PushKey(DIK_SPACE) && fireCoolTime >= kCoolDownTime) {
		fireCoolTime = 0;
		// 弾の速度
		const float kBulletSpeed = 1.0f;
		Vector3 velocity(kBulletSpeed, 0, 0);

		// 速度ベクトルを自機の向きに合わせて回転させる
		//velocity = mathMatrix_->TransformNormal(velocity, transform_.matWorld_);

		// 弾を生成し、初期化
		PlayerBullet* newBullet = new PlayerBullet();
		newBullet->Initialize(bulletModel_, bulletEmitter.translate, velocity);

		// 弾を登録する
		bullets_.push_back(newBullet);
	}
}

void Player::OnCollision()
{
	isDead_ = true;
}

Vector3 Player::GetWorldPosition()
{
	return transform.translate;
}

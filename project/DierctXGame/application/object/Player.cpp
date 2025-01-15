#include "Player.h"

Player::~Player()
{
	for (PlayerBullet* bullet : bullets_) {
		delete bullet;
	}
	delete bulletModel_;
	bullets_.clear();
}

void Player::Initialize(Object3d* model, Object3d* bulletModel)
{
	// 入力の初期化
	input_ = Input::GetInstance();
	model_ = model;

	// 弾のモデルを設定
	bulletModel_ = bulletModel;

	SetInitialize();
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
	if (hp_ <= 0) {
		isDead_ = true;
	}
	// 無敵時間が経過したかどうかを確認
	if (invincible_ && std::chrono::steady_clock::now() >= invincibleEndTime_) {
		invincible_ = false;
	}
	// 弾丸の更新
	for (PlayerBullet* bullet : bullets_) {

		bullet->Update();
	}
	beginTime--;
	if (beginTime <= 0) {
		Attack();
	}
	// 入力の更新
	input_->Update();

	// 弾の発射位置を設定
	bulletEmitter.translate = transform.translate;
	// 操作
	if (input_->PushKey(DIK_RIGHTARROW)) {
		transform.translate.x += 0.08f;

	}
	if (input_->PushKey(DIK_LEFTARROW)) {
		transform.translate.x -= 0.08f;
	}
	if (input_->PushKey(DIK_DOWNARROW)) {
		transform.translate.y -= 0.08f;

	}
	if (input_->PushKey(DIK_UPARROW)) {
		transform.translate.y += 0.08f;
	}
	if (transform.translate.x <= -4.0f) {
		transform.translate.x = -4.0f;
	}
	else if (transform.translate.x >= 4.0f) {
		transform.translate.x = 4.0f;
	}
	if (transform.translate.y <= -2.2f) {
		transform.translate.y = -2.2f;
	}
	else if (transform.translate.y >= 2.2f) {
		transform.translate.y = 2.2f;
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
		const float kBulletSpeed = 0.25f;
		Vector3 velocity(kBulletSpeed, 0, 0);

		// 速度ベクトルを自機の向きに合わせて回転させる
		//velocity = mathMatrix_->TransformNormal(velocity, transform_.matWorld_);

		// 弾を生成し、初期化
		PlayerBullet* newBullet = new PlayerBullet();
		Object3d* model = new Object3d();
		model = bulletModel_;
		bulletModels.push_back(model);
		for (auto& bulletModel : bulletModels) {
			newBullet->Initialize(bulletModel, bulletEmitter.translate, velocity);
		}
		

		// 弾を登録する
		bullets_.push_back(newBullet);
	}
}

void Player::OnCollision()
{
	// 無敵状態でない場合のみ処理を行う
	if (!invincible_) {
		hp_ -= 1;
		// 無敵状態を開始
		invincible_ = true;
		invincibleEndTime_ = std::chrono::steady_clock::now() + invincibleDuration_;
	}
}

Vector3 Player::GetWorldPosition()
{
	return transform.translate;
}

bool Player::IsInvincible() const
{
	return invincible_;
}

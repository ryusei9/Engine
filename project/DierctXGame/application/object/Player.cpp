#include "Player.h"
#include <imGui.h>

Player::~Player()
{
	for (auto& bullet : bullets_) {
		delete bullet;
	}
	
	bullets_.clear();
}

void Player::Initialize(Object3d* model, Object3d* bulletModel)
{
	// 入力の初期化
	input_ = Input::GetInstance();
	model_ = model;
	worldTransform_.Initialize();
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
	for (auto& bullet : bullets_) {

		bullet->Update();
	}
	beginTime--;
	if (beginTime <= 0) {
		Attack();
	}
	// 入力の更新
	input_->Update();

	// 弾の発射位置を設定
	bulletEmitter.translate = worldTransform_.translate_;
	// 操作
	if (input_->PushKey(DIK_RIGHTARROW)) {
		worldTransform_.translate_.x += 0.08f;

	}
	if (input_->PushKey(DIK_LEFTARROW)) {
		worldTransform_.translate_.x -= 0.08f;
	}
	if (input_->PushKey(DIK_DOWNARROW)) {
		worldTransform_.translate_.y -= 0.08f;

	}
	if (input_->PushKey(DIK_UPARROW)) {
		worldTransform_.translate_.y += 0.08f;
	}
	if (worldTransform_.translate_.x <= -4.0f) {
		worldTransform_.translate_.x = -4.0f;
	}
	else if (worldTransform_.translate_.x >= 4.0f) {
		worldTransform_.translate_.x = 4.0f;
	}
	if (worldTransform_.translate_.y <= -2.2f) {
		worldTransform_.translate_.y = -2.2f;
	}
	else if (worldTransform_.translate_.y >= 2.2f) {
		worldTransform_.translate_.y = 2.2f;
	}
	worldTransform_.UpdateMatrix();
	model_->SetTranslate(worldTransform_.translate_);
	model_->SetScale(worldTransform_.scale_);
	model_->SetRotate(worldTransform_.rotate_);

	// モデルの更新
	model_->Update();
}

void Player::Draw()
{
	// 描画処理
	model_->Draw();
	// 弾描画
	for (auto& bullet : bullets_) {
		bullet->Draw();
	}
}

void Player::Attack()
{
	fireCoolTime++;
	
	if (input_->GetInstance()->PushKey(DIK_SPACE) && fireCoolTime >= kCoolDownTime) {
		fireCoolTime = 0;
		// 音声再生
		//Audio::GetInstance()->SoundPlayWave(soundData1);
		// 弾の速度
		const float kBulletSpeed = 0.25f;
		Vector3 velocity(kBulletSpeed, 0, 0);

		// 速度ベクトルを自機の向きに合わせて回転させる
		//velocity = mathMatrix_->worldTransform_Normal(velocity, worldTransform__.matWorld_);

		// 弾を生成し、初期化
		PlayerBullet* newBullet = new PlayerBullet();
		
		newBullet->Initialize(bulletModel_, bulletEmitter.translate, velocity);
		

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
	// ワールド座標を入れる変数
	Vector3 worldPos = {};
	// ワールド行列の平行移動成分を取得(ワールド座標)
	worldPos.x = worldTransform_.translate_.x;
	worldPos.y = worldTransform_.translate_.y;
	worldPos.z = worldTransform_.translate_.z;

	return worldPos;
}

bool Player::IsInvincible() const
{
	return invincible_;
}

void Player::ImGuiDraw()
{
	ImGui::Begin("Player");
	ImGui::Text("Player");
	ImGui::Text("HP: %d", hp_);
	ImGui::Text("Position: (%.2f, %.2f, %.2f)", worldTransform_.translate_.x, worldTransform_.translate_.y, worldTransform_.translate_.z);
	ImGui::Text("Invincible: %s", invincible_ ? "true" : "false");
	ImGui::Text("BeginTime: %f", beginTime);
	ImGui::Text("FireCoolTime: %f", fireCoolTime);
	ImGui::Text("Dead: %s", isDead_ ? "true" : "false");
	ImGui::Separator();
	for (auto& bullet : bullets_) {
		bullet->DrawImGui();
	}
	ImGui::End();
}

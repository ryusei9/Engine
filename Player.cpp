#define NOMINMAX
#include "Player.h"
#include <cassert>
#include <numbers>
#include <algorithm>
#include "ImGuiManager.h"
#include <TextureManager.h>
#include "WinApp.h"

Player::~Player() {
	
	delete mathMatrix_;
	for (PlayerBullet* bullet : bullets_) {
		delete bullet;
	}
	delete reticleModel_;
	delete sprite2DReticle_;
}

void Player::Initialize(Model* model,uint32_t textureHandle,Vector3 position,ViewProjection* viewProjection) {
	// NULLポインタチェック
	assert(model);
	model_ = model;
	worldTransform_.Initialize();
	textureHandle_ = textureHandle;
	viewProjection_ = viewProjection;
	// シングルトンインスタンスを取得する
	input_ = Input::GetInstance();
	mathMatrix_ = new MathMatrix();
	worldTransform_.translation_ = position;
	// 3Dレティクル用トランスフォーム初期化
	worldTransform3DReticle_.Initialize();
	
	// テクスチャ読み込み
	reticleTextureHandle_ = TextureManager::Load("reticle.png");

	reticleModel_ = Model::Create();

	uint32_t textureReticle = TextureManager::Load("reticle.png");

	// スプライト生成
	sprite2DReticle_ = Sprite::Create(textureReticle, Vector2(640.0f, 360.0f), Vector4(1.0f, 1.0f, 1.0f, 255.0f), Vector2(0.5f, 0.5f));
}

void Player::Update() {
	

	// デスフラグの立った弾を削除
	bullets_.remove_if([](PlayerBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});
	// キャラクターの移動ベクトル
	Vector3 move = {0, 0, 0};

	// キャラクターの移動速さ
	const float kCharacterSpeed = 0.2f;

	/*/////////////////////////////
	/// キーボードによる移動処理
	/////////////////////////////*/

	// 押した方向で移動ベクトルを変更(左右)
	//if (input_->PushKey(DIK_LEFT)) {
	//	move.x -= kCharacterSpeed;
	//} else if (input_->PushKey(DIK_RIGHT)) {
	//	move.x += kCharacterSpeed;
	//}

	//// 押した方向で移動ベクトルを変更(上下)
	//if (input_->PushKey(DIK_DOWN)) {
	//	move.y -= kCharacterSpeed;
	//} else if (input_->PushKey(DIK_UP)) {
	//	move.y += kCharacterSpeed;
	//}

	// ゲームパッドの状態を得る変数
	XINPUT_STATE joyState;

	// ゲームパッド状態取得
	if (Input::GetInstance()->GetJoystickState(0, joyState)) {
		move.x += (float)joyState.Gamepad.sThumbLX / SHRT_MAX * kCharacterSpeed;
		move.y += (float)joyState.Gamepad.sThumbLY / SHRT_MAX * kCharacterSpeed;
	}

	// 座標移動(ベクトルの加算)
	worldTransform_.translation_ = mathMatrix_->MathMatrix::Add(worldTransform_.translation_, move);

	// 移動限界座標
	const float kMoveLimitX = 30.0f;
	const float kMoveLimitY = 20.0f;

	// 範囲を超えない処理
	worldTransform_.translation_.x = std::clamp(worldTransform_.translation_.x, -kMoveLimitX, +kMoveLimitX);
	worldTransform_.translation_.y = std::clamp(worldTransform_.translation_.y, -kMoveLimitY, +kMoveLimitY);

	// 旋回
	Rotate();

	// 攻撃処理
	Attack();

	// 弾更新
	for (PlayerBullet* bullet : bullets_){
		bullet->Update();
	}

	// 自機のワールド座標から3Dレティクルのワールド座標を計算
	// 自機から3Dレティクルへの距離
	const float kDistancePlayerTo3DReticle = 50.0f;
	// 自機から３Dレティクルへのオフセット(Z+向き)
	Vector3 offset = {0, 0, 1.0f};
	// 自機のワールド行列の回転を反映
	offset = mathMatrix_->TransformNormal(offset, worldTransform_.matWorld_);
	// ベクトルの長さを整える
	offset = mathMatrix_->Multiply(kDistancePlayerTo3DReticle, mathMatrix_->Normalize(offset));
	// 3Dレティクルの座標を設定
	worldTransform3DReticle_.translation_ = mathMatrix_->Add(GetWorldPosition(), offset);

	//worldTransform3DReticle_.UpdateMatrix();

	//3Dレティクルのワールド座標から2Dレティクルのスクリーン座標を計算
	Vector3 positionReticle = GetReticleWorldPosition();

	// ビューポート行列
	Matrix4x4 matViewport = mathMatrix_->MakeViewportMatrix(0, 0, WinApp::kWindowWidth, WinApp::kWindowHeight, 0, 1);

	// ビュー行列とプロジェクション行列、ビューポート行列を合成する
	Matrix4x4 matViewProjectionViewport = mathMatrix_->Multiply(viewProjection_->matView, mathMatrix_->Multiply(viewProjection_->matProjection, matViewport));

	// ワールド→スクリーン座標変換(ここで3Dから2Dになる)
	//positionReticle = mathMatrix_->Transform(positionReticle, matViewProjectionViewport);

	//// スプライトのレティクルに座標設定
	//sprite2DReticle_->SetPosition(Vector2(positionReticle.x, positionReticle.y));

	//// マウスカーソルのスクリーン座標からワールド座標を取得して3Dレティクル配置
	//POINT mousePosition;

	//// マウス座標(スクリーン座標)を取得する
	//GetCursorPos(&mousePosition);

	//// クライアントエリア座標に変換する
	//HWND hwnd = WinApp::GetInstance()->GetHwnd();
	//ScreenToClient(hwnd, &mousePosition);

	//// マウス座標を2Dレティクルのスプライトに代入する
	//sprite2DReticle_->SetPosition(Vector2((float)mousePosition.x,(float)mousePosition.y));
	
	//  スプライトの現在座標を取得
	Vector2 spritePosition = sprite2DReticle_->GetPosition();

	// XINPUT_STATE joyState;

	// ジョイスティック状態取得
	if (Input::GetInstance()->GetJoystickState(0, joyState)) {
		spritePosition.x += (float)joyState.Gamepad.sThumbRX / SHRT_MAX * 5.0f;
		spritePosition.y -= (float)joyState.Gamepad.sThumbRY / SHRT_MAX * 5.0f;
		// スプライトの座標変更を反映
		sprite2DReticle_->SetPosition(spritePosition);
	}

	// ビュープロジェクションビューポート合成行列
	Matrix4x4 matVPV = mathMatrix_->Multiply(viewProjection_->matView, mathMatrix_->Multiply(viewProjection_->matProjection, matViewport));

	// 合成行列の逆行列を計算する
	Matrix4x4 matInverseVPV = mathMatrix_->Inverse(matVPV);

	// スクリーン座標
	Vector3 posNear = Vector3((float)spritePosition.x, (float)spritePosition.y, 0);
	Vector3 posFar = Vector3((float)spritePosition.x, (float)spritePosition.y, 1);

	// スクリーン座標系からワールド座標系へ
	posNear = mathMatrix_->Transform(posNear, matInverseVPV);
	posFar = mathMatrix_->Transform(posFar ,matInverseVPV);

	// マウスレイの方向
	Vector3 mouseDirection = mathMatrix_->Subtract(posFar, posNear);

	mouseDirection = mathMatrix_->Normalize(mouseDirection);

	// カメラから照準オブジェクトの距離
	const float kDistanceTestObject = 50.0f;
	worldTransform3DReticle_.translation_ = mathMatrix_->Add(posNear, mathMatrix_->Multiply(kDistanceTestObject, mouseDirection));
	worldTransform3DReticle_.UpdateMatrix();

	

	// ワールドトランスフォームの更新
	worldTransform_.UpdateMatrix();
	// 行列を定数バッファに転送
	//worldTransform_.TransferMatrix();

	// キャラクターの座標を画面表示する処理
	#ifdef _DEBUG
	ImGui::Begin("player");
	// ImGui::Text("debugCamera : C\nbulletShot : SPACE");
	// ImGui::DragFloat3("translation", &worldTransform_.translation_.x,0.1f);
	ImGui::Text("2DReticle:(%f,%f)", sprite2DReticle_->GetPosition().x, sprite2DReticle_->GetPosition().y);
	ImGui::Text("Near:(%+.2f,%+.2f,%+.2f)", posNear.x, posNear.y, posNear.z);
	ImGui::Text("Far:(%+.2f,%+.2f,%+.2f)", posFar.x, posFar.y, posFar.z);
	ImGui::Text("3DReticle:(%+.2f,%+.2f,%+.2f)", worldTransform3DReticle_.translation_.x,
		worldTransform3DReticle_.translation_.y, worldTransform3DReticle_.translation_.z);
	ImGui::End();
	#endif
}

void Player::Draw() {
	// 3Dモデルを描画
	model_->Draw(worldTransform_, *viewProjection_,textureHandle_);
	// 弾描画
	for (PlayerBullet* bullet : bullets_) {
		bullet->Draw(*viewProjection_);
	}
	reticleModel_->Draw(worldTransform3DReticle_, *viewProjection_, reticleTextureHandle_);
}

void Player::Rotate() {
	// 回転速さ[ラジアン/frame]
	const float kRotSpeed = 0.02f;

	// 押した方向で移動ベクトルを変更
	if (input_->PushKey(DIK_A)) {
		worldTransform_.rotation_.y -= kRotSpeed;
	} else if (input_->PushKey(DIK_D)) {
		worldTransform_.rotation_.y += kRotSpeed;
	}
}

void Player::Attack() { 
	XINPUT_STATE joyState;
	// ゲームパッド未接続なら何もせず抜ける
	if (!Input::GetInstance()->GetJoystickState(0, joyState)) {
		return;
	}

	// Rトリガーを押していたら
	if (joyState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)  {
		// 弾の速度
		const float kBulletSpeed = 1.0f;
		Vector3 velocity(0, 0, kBulletSpeed);

		// 速度ベクトルを自機の向きに合わせて回転させる
		//velocity = mathMatrix_->TransformNormal(velocity, worldTransform_.matWorld_);

		// 自機から照準オブジェクトへのベクトル
		velocity = mathMatrix_->Subtract(GetReticleWorldPosition(), GetWorldPosition());
		velocity = mathMatrix_->Multiply(kBulletSpeed,mathMatrix_->Normalize(velocity));

		// 弾を生成し、初期化
		PlayerBullet* newBullet = new PlayerBullet();
		newBullet->Initialize(model_, GetWorldPosition(), velocity);

		// 弾を登録する
		bullets_.push_back(newBullet);
	}
}

void Player::SetParent(const WorldTransform* parent) {
	// 親子関係を結ぶ
	worldTransform_.parent_ = parent;
}

Vector3 Player::GetWorldPosition() { 
	// ワールド座標を入れる変数
	Vector3 worldPos = {};
	// ワールド行列の平行移動成分を取得(ワールド座標)
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

Vector3 Player::GetReticleWorldPosition() { 
	// ワールド座標を入れる変数
	Vector3 worldPos = {};
	// ワールド行列の平行移動成分を取得(ワールド座標)
	worldPos.x = worldTransform3DReticle_.matWorld_.m[3][0];
	worldPos.y = worldTransform3DReticle_.matWorld_.m[3][1];
	worldPos.z = worldTransform3DReticle_.matWorld_.m[3][2];

	return worldPos;
}

void Player::OnCollision() {}

void Player::DrawUI() { sprite2DReticle_->Draw(); }

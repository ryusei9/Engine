#include "RailCamera.h"
#include <ImGuiManager.h>
RailCamera::~RailCamera() { delete mathMatrix_; }

void RailCamera::Initialize(Vector3 translation, Vector3 rotate) {
	// ワールドトランスフォームの初期設定
	worldTransform_.translation_ = translation;
	worldTransform_.rotation_ = rotate;

	// ビュープロジェクションの初期化
	viewProjection_.farZ = 1000.0f;
	viewProjection_.Initialize();
}

void RailCamera::Update() {
	// カメラの座標を画面表示する処理
#ifdef _DEBUG
	ImGui::Begin("Camera");
	ImGui::SliderFloat3("translation", &worldTransform_.translation_.x, -100.0f, 100.0f);
	ImGui::SliderFloat3("rotation", &worldTransform_.rotation_.x, -10.0f, 10.0f);
	ImGui::End();
#endif
	// ワールドトランスフォームの数値を加算(移動)
	// worldTransform_.translation_.z -= 0.1f;
	// ワールドトランスフォームの数値を加算(回転)
	// worldTransform_.rotation_.y += 0.001f;

	// ワールド行列の再計算
	worldTransform_.matWorld_ = mathMatrix_->MakeAffine(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	// カメラオブジェクトのワールド行列からビュー行列を計算する
	viewProjection_.matView = mathMatrix_->Inverse(worldTransform_.matWorld_);
}

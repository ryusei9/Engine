#include "Skydome.h"
#include <imgui.h>
#include <Object3dCommon.h>

void Skydome::Initialize() {
	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();
	// スカイドームのモデルを読み込む
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize("player.obj");
	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.scale_ = { 100.0f, 100.0f, 100.0f }; // スカイドームのスケールを大きく設定
}

void Skydome::Update() {
	// ワールド変換の更新
	worldTransform_.Update();
	object3d_->SetCamera(camera_);
	object3d_->SetTranslate(worldTransform_.translate_);
	object3d_->SetRotate(worldTransform_.rotate_);
	object3d_->SetScale(worldTransform_.scale_);
	object3d_->Update();
}

void Skydome::Draw() {
	// 3Dオブジェクトの描画
	object3d_->Draw();
}

void Skydome::DrawImGui() {
	// ImGuiでの設定
	ImGui::Begin("Skydome Settings");
	ImGui::DragFloat3("Translate", &worldTransform_.translate_.x, 0.01f);
	ImGui::DragFloat3("Rotate", &worldTransform_.rotate_.x, 0.01f);
	ImGui::DragFloat3("Scale", &worldTransform_.scale_.x, 0.01f);
	ImGui::End();
}
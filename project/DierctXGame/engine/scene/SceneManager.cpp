#include "SceneManager.h"
#include "DirectXCommon.h"
#include <imgui.h>
 

SceneManager::~SceneManager()
{
	// 現在のシーンの終了処理
	if (nowScene_) {
		nowScene_->Finalize();
	}
}

void SceneManager::Initialize(WinApp* winApp)
{
	directXCommon_ = DirectXCommon::GetInstance();
	winApp_ = winApp;

	// 初期シーンを設定（例としてDebugSceneを設定）
	nowScene_ = std::make_unique<TitleScene>();
	nowScene_->Initialize(directXCommon_, winApp_);

	// シーンの初期設定
	currentSceneNo_ = 0;
	prevSceneNo_ = -1;
}

void SceneManager::Update()
{
	prevSceneNo_ = currentSceneNo_;
	currentSceneNo_ = nowScene_->GetSceneNo();

	// 次のシーンがある場合
	if (prevSceneNo_ != currentSceneNo_) {
		//---------------------------------------
		// 旧シーンの終了処理
		if (nowScene_ != nullptr) {
			nowScene_->Finalize();
		}
		//---------------------------------------
		// 新シーンの初期化
		if (currentSceneNo_ == GAMEPLAY) {
			nowScene_ = std::make_unique<GamePlayScene>();
		} else if (currentSceneNo_ == TITLE) {
			nowScene_ = std::make_unique<TitleScene>();
		}
		// シーンの初期化
		nowScene_->Initialize(directXCommon_, winApp_);
	}

	//========================================
	// シーンの更新
	if (nowScene_) {
		nowScene_->Update();
	}
}

void SceneManager::Draw()
{
	// 実行中シーンを描画
	nowScene_->Draw();
}

void SceneManager::DrawImGui()
{
	if (nowScene_) {
		nowScene_->DrawImGui();
	}
	/*ImGui::Begin("DxCommon");
	ImGui::SliderFloat("Dissolve Threshold", &directXCommon_->GetDissolveParam()->threshold, 0.0f, 1.0f);
	ImGui::SliderFloat("Edge Width", &directXCommon_->GetDissolveParam()->edgeWidth, 0.0f, 0.2f);
	ImGui::SliderFloat("Edge Strength", &directXCommon_->GetDissolveParam()->edgeStrength, 0.0f, 5.0f);
	ImGui::ColorEdit3("Edge Color", directXCommon_->GetDissolveParam()->edgeColor);
	ImGui::End();*/
}

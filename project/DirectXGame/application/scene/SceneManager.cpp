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

	// 初期シーンを設定
	nowScene_ = std::make_unique<TitleScene>();
	nowScene_->Initialize(directXCommon_, winApp_);

	// シーンの初期設定（定数化）
	currentSceneNo_ = SceneDefaults::kInitialSceneNo;
	prevSceneNo_ = SceneDefaults::kNoPrevSceneNo;
}

void SceneManager::Update()
{
	// シーン遷移のチェック
	if (nowScene_) {
		// 次のシーンが要求されているかチェック
		std::optional<int32_t> nextScene = nowScene_->GetNextScene();
		
		if (nextScene.has_value()) {
			// シーン番号を更新
			prevSceneNo_ = currentSceneNo_;
			currentSceneNo_ = nextScene.value();
			
			//---------------------------------------
			// 旧シーンの終了処理
			nowScene_->Finalize();
			
			//---------------------------------------
			// 新シーンの生成と初期化
			if (currentSceneNo_ == GAMEPLAY) {
				nowScene_ = std::make_unique<GamePlayScene>();
			} else if (currentSceneNo_ == TITLE) {
				nowScene_ = std::make_unique<TitleScene>();
			} else if (currentSceneNo_ == GAMEOVER) {
				nowScene_ = std::make_unique<GameOverScene>();
			}
			
			// 新シーンの初期化
			if (nowScene_) {
				nowScene_->Initialize(directXCommon_, winApp_);
			}
		}
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
	if (nowScene_) {
		nowScene_->Draw();
	}
}

void SceneManager::DrawImGui()
{
	if (nowScene_) {
		nowScene_->DrawImGui();
	}
}

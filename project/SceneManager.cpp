#include "SceneManager.h"

SceneManager::~SceneManager()
{
	// 現在のシーンの終了処理
	if (scene_) {
		scene_->Finalize();
		delete scene_;
	}
}

void SceneManager::Initialize(SpriteCommon* spriteCommon, DirectXCommon* directXCommon, WinApp* winApp)
{
	spriteCommon_ = spriteCommon;
	directXCommon_ = directXCommon;
	winApp_ = winApp;
}

void SceneManager::Update()
{
	// シーン切り替え
	// 次シーンの予約があるなら
	if (nextScene_) {
		// 現在のシーンの終了処理
		if (scene_) {
			scene_->Finalize();
			delete scene_;
		}
		// 次のシーンの初期化
		scene_ = nextScene_;
		scene_->Initialize(spriteCommon_, directXCommon_, winApp_);
		nextScene_ = nullptr;
	}
	// 実行中シーンを更新
	scene_->Update();
}

void SceneManager::Draw()
{
	// 実行中シーンを描画
	scene_->Draw();
}

#pragma once
#include <BaseScene.h>
#include <SpriteCommon.h>
#include <DirectXCommon.h>
#include <WinApp.h>
#include <TitleScene.h>
#include <GamePlayScene.h>
// シーン管理
class SceneManager
{
public:
	// デストラクタ
	~SceneManager();

	// 初期化
	void Initialize(SpriteCommon* spriteCommon, DirectXCommon* directXCommon, WinApp* winApp);

	// 更新
	void Update();

	// 描画
	void Draw();

	
private:

	// 現在のシーン
	std::unique_ptr<BaseScene> nowScene_ = nullptr;

	// 次のシーン
	std::unique_ptr<BaseScene> nextScene_ = nullptr;

	// スプライトコモン
	SpriteCommon* spriteCommon_ = nullptr;

	// ダイレクトXコモン
	DirectXCommon* directXCommon_ = nullptr;

	// WinApp
	WinApp* winApp_ = nullptr;

	// シーンの管理
	int currentSceneNo_;
	int prevSceneNo_;
};


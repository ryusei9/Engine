#pragma once
#include <BaseScene.h>
#include <SpriteCommon.h>
#include <DirectXCommon.h>
#include <WinApp.h>
#include <TitleScene.h>
#include <GamePlayScene.h>

/// <summary>
// シーン管理
/// </summary>
class SceneManager
{
public:
	// デストラクタ
	~SceneManager();

	// 初期化
	void Initialize(WinApp* winApp);

	// 更新
	void Update();

	// 描画
	void Draw();

	// ImGui描画
	void DrawImGui();
	
private:

	// 現在のシーン
	std::unique_ptr<BaseScene> nowScene_ = nullptr;

	// 次のシーン
	std::unique_ptr<BaseScene> nextScene_ = nullptr;

	// ダイレクトXコモン
	DirectXCommon* directXCommon_ = nullptr;

	// WinApp
	WinApp* winApp_ = nullptr;

	// シーンの管理
	int currentSceneNo_;
	int prevSceneNo_;
};


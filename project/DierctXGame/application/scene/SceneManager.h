#pragma once
#include <BaseScene.h>
#include <SpriteCommon.h>
#include <DirectXCommon.h>
#include <WinApp.h>
#include <TitleScene.h>
#include <GamePlayScene.h>
#include <GameOverScene.h>

/// <summary>
/// シーン管理
/// </summary>
namespace SceneDefaults {
	// 初期シーン番号（TITLE）
	inline constexpr int32_t kInitialSceneNo = TITLE;
	// 未設定の過去シーン番号
	inline constexpr int32_t kNoPrevSceneNo  = -1;
}

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

	// 次のシーン（未使用なら保持しない）
	std::unique_ptr<BaseScene> nextScene_ = nullptr;

	// ダイレクトXコモン
	DirectXCommon* directXCommon_ = nullptr;

	// WinApp
	WinApp* winApp_ = nullptr;

	// シーンの管理
	int32_t currentSceneNo_ = SceneDefaults::kInitialSceneNo;
	int32_t prevSceneNo_    = SceneDefaults::kNoPrevSceneNo;
};


#pragma once
#include <BaseScene.h>
#include <SpriteCommon.h>
#include <DirectXCommon.h>
#include <WinApp.h>
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

	// シーンの変更
	void SetNextScene(BaseScene* nextScene) { nextScene_ = nextScene; }
private:
	// 現在のシーン
	BaseScene* scene_ = nullptr;

	// 次のシーン
	BaseScene* nextScene_ = nullptr;

	// スプライトコモン
	SpriteCommon* spriteCommon_ = nullptr;

	// ダイレクトXコモン
	DirectXCommon* directXCommon_ = nullptr;

	// WinApp
	WinApp* winApp_ = nullptr;
};


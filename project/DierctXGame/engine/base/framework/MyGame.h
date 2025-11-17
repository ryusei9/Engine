#pragma once
#include "SRFramework.h"
#include "GamePlayScene.h"
#include "TitleScene.h"
#include "ImGuiManager.h"

/// <summary>
/// ゲーム全体
/// </summary>
class MyGame : public SRFramework
{
public:

	// 初期化
	void Initialize() override;

	// 終了
	void Finelize() override;

	// 毎フレーム更新
	void Update() override;

	// 描画
	void Draw() override;


private:
	// ゲームプレイシーン
	GamePlayScene* scene_;
	// タイトルシーン
	TitleScene* titleScene_;
};


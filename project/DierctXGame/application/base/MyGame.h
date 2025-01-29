#pragma once
#include "SRFramework.h"
#include "GamePlayeScene.h"

// ゲーム全体
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
	GamePlayeScene* scene_;
	

	// ゲーム終了フラグ
	//bool endRequest_ = false;
};


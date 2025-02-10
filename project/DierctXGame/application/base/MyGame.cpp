#include "MyGame.h"


void MyGame::Initialize()
{
	// 基底クラスの初期化処理
	SRFramework::Initialize();
	//titleScene_ = new TitleScene();
	//// ゲームプレイシーンの初期化
	//titleScene_->Initialize(SRFramework::GetSpriteCommon(), SRFramework::GetDirectXCommon(), SRFramework::GetWinApp());
}

void MyGame::Finelize()
{
	// ゲームプレイシーンの終了処理
	//titleScene_->Finalize();
	//// ゲームプレイシーンの破棄
	//delete titleScene_;
	// 基底クラスの終了処理
	SRFramework::Finelize();
}

void MyGame::Update()
{
	SRFramework::Update();
	// ゲームプレイシーンの更新
	//titleScene_->Update();
}

void MyGame::Draw()
{
	SRFramework::PreDraw();

	SRFramework::PreDrawObject3d();

	SRFramework::PreDrawSprite();
	
	SRFramework::PostDraw();
}

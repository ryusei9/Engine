#include "MyGame.h"


void MyGame::Initialize()
{
	// 基底クラスの初期化処理
	SRFramework::Initialize();
}

void MyGame::Finelize()
{
	SRFramework::Finelize();
}

void MyGame::Update()
{
	SRFramework::Update();
	// ImGuiの更新
	imGuiManager->Begin();
#ifdef _DEBUG
	// ゲームプレイシーンの更新
	sceneManager_->DrawImGui();

#endif
	imGuiManager->End();
	
}

void MyGame::Draw()
{
	SRFramework::PrePostEffect();

	sceneManager_->Draw();

	ParticleManager::GetInstance()->Draw();

	SRFramework::PreDraw();

	SRFramework::DrawPostEffect();

	imGuiManager->Draw();
	
	SRFramework::PostDraw();
}

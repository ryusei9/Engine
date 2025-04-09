#include "TitleScene.h"
#include <imgui.h>

void TitleScene::Initialize(DirectXCommon* directXCommon, WinApp* winApp)
{
	sprite = std::make_unique<Sprite>();
	sprite->Initialize(directXCommon, "resources/mori_Red.png");
	input = std::make_unique<Input>();
	input->Initialize(winApp);
	Audio::GetInstance()->Initialize();
	soundData1 = Audio::GetInstance()->SoundLoadWave("resources/Alarm01.wav");
	// 音声再生
	Audio::GetInstance()->SoundPlayWave(soundData1);
}

void TitleScene::Update()
{
	// 入力の更新
	input->Update();

	// エンターキーでゲームシーンに切り替える
	if (input->TriggerKey(DIK_RETURN))
	{
		SetSceneNo(GAMEPLAY);
	}

	sprite->Update();
	sprite->SetPosition(spritePosition);
}

void TitleScene::Draw()
{
	sprite->Draw();
}

void TitleScene::Finalize()
{
	Audio::GetInstance()->SoundUnload(&soundData1);
	Audio::GetInstance()->Finalize();
}

void TitleScene::DrawImGui()
{
	ImGui::Begin("TitleScene");
	
	ImGui::End();
}

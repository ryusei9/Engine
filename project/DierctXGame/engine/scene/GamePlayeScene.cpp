#include "GamePlayeScene.h"
#include "SRFramework.h"
void GamePlayeScene::Initialize(SpriteCommon* spriteCommon, DirectXCommon* directXCommon, WinApp* winApp)
{
	sprite->Initialize(spriteCommon, directXCommon, "resources/mori.png");
	input->Initialize(winApp);
	Audio::GetInstance()->Initialize();
	soundData1 = Audio::GetInstance()->SoundLoadWave("resources/Alarm01.wav");
	// 音声再生
	Audio::GetInstance()->SoundPlayWave(soundData1);
}

void GamePlayeScene::Update()
{
	// 入力の更新
	input->Update();


	sprite->Update();
	sprite->SetPosition(spritePosition);
}

void GamePlayeScene::Draw()
{
	sprite->Draw();
}

void GamePlayeScene::Finalize()
{
	delete sprite;
	delete input;
	Audio::GetInstance()->SoundUnload(&soundData1);
	Audio::GetInstance()->Finalize();
}

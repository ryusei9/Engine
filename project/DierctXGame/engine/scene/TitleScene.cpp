#include "TitleScene.h"

void TitleScene::Initialize(SpriteCommon* spriteCommon, DirectXCommon* directXCommon, WinApp* winApp)
{
	sprite->Initialize(spriteCommon, directXCommon, "resources/mori_Red.png");
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


	sprite->Update();
	sprite->SetPosition(spritePosition);
}

void TitleScene::Draw()
{
	sprite->Draw();
}

void TitleScene::Finalize()
{
	delete sprite;
	delete input;
	Audio::GetInstance()->SoundUnload(&soundData1);
	Audio::GetInstance()->Finalize();
}

#include "GamePlayScene.h"
#include "SRFramework.h"
void GamePlayScene::Initialize(SpriteCommon* spriteCommon, DirectXCommon* directXCommon, WinApp* winApp)
{
	sprite->Initialize(spriteCommon, directXCommon, "resources/mori.png");
	input->Initialize(winApp);
	Audio::GetInstance()->Initialize();
	soundData1 = Audio::GetInstance()->SoundLoadWave("resources/Alarm01.wav");
	// 音声再生
	Audio::GetInstance()->SoundPlayWave(soundData1);
}

void GamePlayScene::Update()
{
	// 入力の更新
	input->Update();

	// エンターキーでタイトルシーンに切り替える
	if (input->TriggerKey(DIK_RETURN))
	{
		SetSceneNo(TITLE);
	}

	sprite->Update();
	sprite->SetPosition(spritePosition);
}

void GamePlayScene::Draw()
{
	sprite->Draw();
}

void GamePlayScene::Finalize()
{
	delete sprite;
	delete input;
	Audio::GetInstance()->SoundUnload(&soundData1);
	Audio::GetInstance()->Finalize();
}

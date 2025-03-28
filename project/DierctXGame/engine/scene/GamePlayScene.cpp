#include "GamePlayScene.h"
#include "SRFramework.h"
void GamePlayScene::Initialize(SpriteCommon* spriteCommon, DirectXCommon* directXCommon, WinApp* winApp)
{
	sprite = std::make_unique<Sprite>();
	input = std::make_unique<Input>();
	sprite->Initialize(spriteCommon, directXCommon, "resources/mori.png");
	input->Initialize(winApp);
	Audio::GetInstance()->Initialize();
	soundData1 = Audio::GetInstance()->SoundLoadWave("resources/Alarm01.wav");
	// 音声再生
	Audio::GetInstance()->SoundPlayWave(soundData1);

	particleManager = std::make_shared<ParticleManager>();
	particleManager->GetInstance()->CreateParticleGroup("test", "resources/mori.png");
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
	particleManager->Emit("test", { 0.0f,0.0f,0.0f }, 10);
	sprite->Update();
	sprite->SetPosition(spritePosition);
}

void GamePlayScene::Draw()
{
	sprite->Draw();
}

void GamePlayScene::Finalize()
{
	Audio::GetInstance()->SoundUnload(&soundData1);
	Audio::GetInstance()->Finalize();
}

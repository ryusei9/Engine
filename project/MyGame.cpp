#include "MyGame.h"


void MyGame::Initialize()
{
	// 基底クラスの初期化処理
	SRFramework::Initialize();
	sprite->Initialize(SRFramework::GetSpriteCommon(), SRFramework::GetDirectXCommon(), "resources/mori.png");
	input->Initialize(SRFramework::GetWinApp());
	Audio::GetInstance()->Initialize();
	soundData1 = Audio::GetInstance()->SoundLoadWave("resources/Alarm01.wav");
	// 音声再生
	Audio::GetInstance()->SoundPlayWave(soundData1);
}

void MyGame::Finelize()
{
	delete sprite;
	delete input;
	Audio::GetInstance()->SoundUnload(&soundData1);
	Audio::GetInstance()->Finalize();
	SRFramework::Finelize();
}

void MyGame::Update()
{
	SRFramework::Update();
	// 入力の更新
	input->Update();
	

	sprite->Update();
	sprite->SetPosition(spritePosition);
}

void MyGame::Draw()
{
	SRFramework::PreDraw();
	sprite->Draw();
	SRFramework::PostDraw();
}

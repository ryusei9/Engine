#include "GamePlayScene.h"
#include "SRFramework.h"
void GamePlayScene::Initialize(SpriteCommon* spriteCommon, DirectXCommon* directXCommon, WinApp* winApp)
{
	sprite = std::make_unique<Sprite>();
	input = std::make_unique<Input>();
	// テクスチャ"モリ"を使用
	sprite->Initialize(spriteCommon, directXCommon, "resources/mori.png");
	input->Initialize(winApp);
	Audio::GetInstance()->Initialize();
	soundData1 = Audio::GetInstance()->SoundLoadWave("resources/Alarm01.wav");
	// 音声再生
	Audio::GetInstance()->SoundPlayWave(soundData1);

	// パーティクルマネージャの初期化
	particleManager = ParticleManager::GetInstance();
	// テクスチャ"モリ"を使用
	particleManager->GetInstance()->CreateParticleGroup("mori", "resources/mori.png");
	// テクスチャ"UV"を使用
	particleManager->GetInstance()->CreateParticleGroup("uv", "resources/uvChecker.png");

	particleEmitter1 = std::make_unique<ParticleEmitter>(particleManager,"mori");
	particleEmitter2 = std::make_unique<ParticleEmitter>(particleManager, "uv");
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
	// パーティクルグループ"モリ"の更新
	particleEmitter1->SetPosition(particlePosition1);
	particleEmitter1->SetParticleRate(100);
	particleEmitter1->Update();

	// パーティクルグループ"UV"の更新
	particleEmitter2->SetPosition(particlePosition2);
	particleEmitter2->SetParticleRate(100);
	particleEmitter2->Update();

	// スプライトの更新
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

void GamePlayScene::DrawImGui()
{
	ImGui::Begin("GamePlayScene");
	// パーティクルエミッター1の位置
	ImGui::SliderFloat3("ParticleEmitter1 Position",&particlePosition1.x,-10.0f,50.0f);
	ImGui::SliderFloat3("ParticleEmitter2 Position", &particlePosition2.x, -10.0f, 50.0f);
	ImGui::End();
}

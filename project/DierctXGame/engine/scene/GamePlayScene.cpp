#include "GamePlayScene.h"
#include "SRFramework.h"
void GamePlayScene::Initialize(DirectXCommon* directXCommon, WinApp* winApp)
{
	sprite = std::make_unique<Sprite>();
	input = std::make_unique<Input>();
	// テクスチャ"モリ"を使用
	sprite->Initialize(directXCommon, "resources/gradationLine.png");
	input->Initialize(winApp);
	Audio::GetInstance()->Initialize();
	soundData1 = Audio::GetInstance()->SoundLoadWave("resources/Alarm01.wav");
	// 音声再生
	Audio::GetInstance()->SoundPlayWave(soundData1);

	// パーティクルマネージャの初期化
	particleManager = ParticleManager::GetInstance();

	// テクスチャ"モリ"を使用
	particleManager->GetInstance()->CreateParticleGroup("mori", "resources/gradationLine.png");
	// テクスチャ"UV"を使用
	particleManager->GetInstance()->CreateParticleGroup("uv", "resources/uvChecker.png");

	particleEmitter1 = std::make_unique<ParticleEmitter>(particleManager,"mori");
	

	//particleEmitter2 = std::make_unique<ParticleEmitter>(particleManager, "uv");

	//particleEmitter2->SetUseRingParticle(false);

	// ボールの初期化
	ball = std::make_unique<Object3d>();
	ball->Initialize("monsterBall.obj");

	ballTransform = {
		{1.0f,1.0f,1.0f}, // スケール
		{0.0f,0.0f,0.0f}, // 回転
		{0.0f,0.0f,5.0f}  // 座標
	};
	ball->SetWorldTransform(ballTransform);

	// ボールの初期化
	ground = std::make_unique<Object3d>();
	ground->Initialize("plane.obj");

	groundTransform = {
		{1.0f,1.0f,1.0f}, // スケール
		{0.0f,0.0f,0.0f}, // 回転
		{0.0f,0.0f,5.0f}  // 座標
	};
	ground->SetTransform(groundTransform);
}

void GamePlayScene::Update()
{
	// 入力の更新
	input->Update();

	// エンターキーでタイトルシーンに切り替える
	/*if (input->TriggerKey(DIK_RETURN))
	{
		SetSceneNo(TITLE);
	}*/


	// パーティクルグループ"モリ"の更新
	particleEmitter1->SetPosition(particlePosition1);
	particleEmitter1->SetParticleRate(1);
	particleEmitter1->Update();

	//// パーティクルグループ"UV"の更新
	/*particleEmitter2->SetPosition(particlePosition2);
	particleEmitter2->SetParticleRate(8);
	particleEmitter2->Update();*/


	// スプライトの更新
	/*sprite->Update();
	sprite->SetPosition(spritePosition);*/

	/*------オブジェクトの更新------*/
	// ボールの更新
	/*ball->Update();
	ball->SetTransform(ballTransform);
	ground->Update();
	ground->SetTransform(groundTransform);*/
}

void GamePlayScene::Draw()
{
	/*------スプライトの更新------*/
	SpriteCommon::GetInstance()->DrawSettings();
	//sprite->Draw();

	/*------オブジェクトの描画------*/
	Object3dCommon::GetInstance()->DrawSettings();
	// ボールの描画
	/*ball->Draw();
	ground->Draw();*/
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
	/*ImGui::SliderFloat3("ParticleEmitter2 Position", &particlePosition2.x, -10.0f, 50.0f);*/
	// ボールの座標
	//ImGui::SliderFloat3("Ball Position", &ballTransform.translate.x, -10.0f, 50.0f);
	//// ボールのスケール
	//ImGui::SliderFloat3("Ball Scale", &ballTransform.scale.x, 0.0f, 10.0f);
	//// ボールの回転
	//ImGui::SliderFloat3("Ball Rotate", &ballTransform.rotate.x, 0.0f, 360.0f);

	//// ボールの座標
	//ImGui::SliderFloat3("Ball Position", &groundTransform.translate.x, -10.0f, 50.0f);
	//// ボールのスケール
	//ImGui::SliderFloat3("Ball Scale", &groundTransform.scale.x, 0.0f, 10.0f);
	//// ボールの回転
	//ImGui::SliderFloat3("Ball Rotate", &groundTransform.rotate.x, 0.0f, 360.0f);

	/*ball->DrawImGui();
	ground->DrawImGui();*/
	ImGui::End();
}

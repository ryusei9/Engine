#include "GamePlayScene.h"
#include "SRFramework.h"

void GamePlayScene::Initialize(DirectXCommon* directXCommon, WinApp* winApp)
{
	sprite = std::make_unique<Sprite>();
	input = Input::GetInstance();
	// テクスチャ"モリ"を使用
	sprite->Initialize(directXCommon, "resources/gradationLine.png");
	
	Audio::GetInstance()->Initialize();
	soundData1 = Audio::GetInstance()->SoundLoadWave("resources/Alarm01.wav");
	// 音声再生
	Audio::GetInstance()->SoundPlayWave(soundData1);

	// パーティクルマネージャの初期化
	particleManager = ParticleManager::GetInstance();

	// テクスチャ"モリ"を使用
	particleManager->GetInstance()->CreateParticleGroup("mori", "resources/circle2.png");
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
	ball->SetTransform(ballTransform);

	// ボールの初期化
	ground = std::make_unique<Object3d>();
	ground->Initialize("plane.obj");

	groundTransform = {
		{1.0f,1.0f,1.0f}, // スケール
		{0.0f,0.0f,0.0f}, // 回転
		{0.0f,0.0f,5.0f}  // 座標
	};
	ground->SetTransform(groundTransform);

	// プレイヤーの初期化
	player_ = std::make_unique<Player>();
	player_->Initialize();
	//player_->SetBullet(playerBullet_.get());

	// 敵の初期化
	enemy_ = std::make_unique<Enemy>();
	enemy_->Initialize();

	playerBullets_ = &player_->GetBullets();
	// プレイヤーの弾の初期化
	/*playerBullet_ = std::make_unique<PlayerBullet>();
	playerBullet_->Initialize();
	playerBullet_->SetPlayer(player_.get());*/

	// 衝突マネージャの生成
	collisionManager_ = std::make_unique<CollisionManager>();
	collisionManager_->Initialize();
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
	// プレイヤーの更新
	player_->Update();

	// 敵の更新
	enemy_->Update();

	// パーティクルグループ"モリ"の更新
	/*particleEmitter1->SetPosition(particlePosition1);
	particleEmitter1->SetParticleRate(8);
	particleEmitter1->Update();*/

	// 衝突マネージャの更新
	collisionManager_->Update();
	CheckAllCollisions();// 衝突判定と応答
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
	// プレイヤーの描画
	player_->Draw();

	// 敵の描画
	enemy_->Draw();
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

void GamePlayScene::CheckAllCollisions()
{
	// 衝突マネージャのリセット
	collisionManager_->Reset();

	// コライダーをリストに登録
	collisionManager_->AddCollider(enemy_.get());

	// 複数についてコライダーをリストに登録
	for (const auto& bullet : *playerBullets_)
	{
		collisionManager_->AddCollider(bullet.get());
	}
	// 衝突判定と応答
	collisionManager_->CheckCollision();
}

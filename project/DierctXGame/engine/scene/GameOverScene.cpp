#include "GameOverScene.h"
#include <imgui.h>

void GameOverScene::Initialize(DirectXCommon* directXCommon, WinApp* winApp)
{
	// 背景スプライトの生成
	//backgroundSprite_ = std::make_unique<Sprite>();
	//backgroundSprite_->Initialize(directXCommon, "resources/Black.png");

	input = std::make_unique<Input>();
	input->Initialize(winApp);

	fadeManager_ = std::make_unique<FadeManager>();
	fadeManager_->Initialize();
	fadeManager_->FadeOutStart(0.02f);

	gameOverText_ = std::make_unique<Object3d>();
	gameOverText_->Initialize("GameOver.obj");

	cameraManager_ = std::make_unique<CameraManager>();
	cameraManager_->Initialize(Object3dCommon::GetInstance()->GetDefaultCamera());

	cameraManager_->SetCameraPosition({ 0.0f,1.0f,-10.0f });
	cameraManager_->SetCameraRotation({ 0.1f,0.0f,0.0f });

	gameOverTextTransform_.Initialize();
	gameOverTextTransform_.translate_ = { 0.0f,1.4f,0.0f };
	gameOverTextTransform_.rotate_ = { 0.0f,1.6f,0.0f };

	gameOverGuideTransform_.Initialize();
	gameOverGuideTransform_.translate_ = { 0.0f,-2.5f,4.0f };
	gameOverGuideTransform_.rotate_ = { 0.0f,1.6f,0.0f };

	gameOverGuide_ = std::make_unique<Object3d>();
	gameOverGuide_->Initialize("GameOverGuide.obj");
	

	skydome_ = std::make_unique<Object3d>();
	skydome_->Initialize("skydome.obj");
	skydome_->SetPointLight(0.0f);
	skydomeTransform_.Initialize();

	player_ = std::make_unique<Object3d>();
	player_->Initialize("Player.obj");

	playerTransform_.Initialize();
	playerTransform_.translate_ = { 0.0f,3.0f,0.0f };
	playerTransform_.rotate_ = { 0.0f,0.0f,-1.0f };
}

void GameOverScene::Update()
{
	// 入力の更新
	input->Update();

	// スペースキーでタイトルシーンに切り替える
	if(input->TriggerKey(DIK_SPACE))
	{
		returnToTitle_ = true;
	}

	if (returnToTitle_ && !fadeStarted_) {
		fadeManager_->FadeInStart(0.02f, [this]() {
			SetSceneNo(TITLE);
			});
		fadeStarted_ = true;
	}
	//backgroundSprite_->Update();
	fadeManager_->Update();

	gameOverText_->SetWorldTransform(gameOverTextTransform_);
	gameOverText_->Update();

	skydome_->SetWorldTransform(skydomeTransform_);
	skydome_->Update();

	gameOverGuide_->SetWorldTransform(gameOverGuideTransform_);
	gameOverGuide_->Update();

	// --- プレイヤー落下＆回転処理を追加 ---
	// ゆっくり落下
	playerTransform_.translate_.y -= 0.01f; // 落下速度（調整可）
	// ゆっくり回転
	playerTransform_.rotate_.x += 0.02f;    // X軸回転（調整可）
	

	player_->SetWorldTransform(playerTransform_);
	player_->Update();

	cameraManager_->Update();
	Object3dCommon::GetInstance()->SetDefaultCamera(cameraManager_->GetMainCamera());
}

void GameOverScene::Draw()
{
	
	Object3dCommon::GetInstance()->DrawSettings();
	gameOverText_->Draw();
	gameOverGuide_->Draw();
	player_->Draw();
	skydome_->Draw();
	/*------スプライトの更新------*/
	SpriteCommon::GetInstance()->DrawSettings();
	//backgroundSprite_->Draw();
	fadeManager_->Draw();
}

void GameOverScene::Finalize()
{
}

void GameOverScene::DrawImGui()
{
#ifdef USE_IMGUI
	ImGui::Begin("GameOverScene");
	ImGui::DragFloat3("text_translate_", &gameOverTextTransform_.translate_.x);
	ImGui::DragFloat3("text_rotate_", &gameOverTextTransform_.rotate_.x);
	ImGui::DragFloat3("guide_translate_", &gameOverGuideTransform_.translate_.x);
	ImGui::DragFloat3("guide_rotate_", &gameOverGuideTransform_.rotate_.x);
	ImGui::DragFloat3("guide_scale_", &gameOverGuideTransform_.scale_.x);
	ImGui::DragFloat3("player_translate_", &playerTransform_.translate_.x);
	ImGui::DragFloat3("player_rotate_", &playerTransform_.rotate_.x);
	ImGui::End();
	cameraManager_->DrawImGui();
#endif
}

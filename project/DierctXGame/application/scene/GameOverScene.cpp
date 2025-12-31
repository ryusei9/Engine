#include "GameOverScene.h"
#include <imgui.h>

void GameOverScene::Initialize(DirectXCommon* /*directXCommon*/, WinApp* winApp)
{
	// インプット
	input_ = std::make_unique<Input>();
	input_->Initialize(winApp);

	// フェード
	fadeManager_ = std::make_unique<FadeManager>();
	fadeManager_->Initialize();
	fadeManager_->FadeOutStart(GameOverDefaults::kFadeStep);

	// 3Dオブジェクト
	gameOverText_ = std::make_unique<Object3d>();
	gameOverText_->Initialize("GameOver.obj");

	// カメラ
	cameraManager_ = std::make_unique<CameraManager>();
	cameraManager_->Initialize(Object3dCommon::GetInstance()->GetDefaultCamera());
	cameraManager_->SetCameraPosition(GameOverDefaults::kCamPos);
	cameraManager_->SetCameraRotation(GameOverDefaults::kCamRot);

	// Transform初期化
	gameOverTextTransform_.Initialize();
	gameOverTextTransform_.SetTranslate(GameOverDefaults::kTextTranslate);
	gameOverTextTransform_.SetRotate(GameOverDefaults::kTextRotate);

	gameOverGuideTransform_.Initialize();
	gameOverGuideTransform_.SetTranslate(GameOverDefaults::kGuideTranslate);
	gameOverGuideTransform_.SetRotate(GameOverDefaults::kGuideRotate);

	// ガイド
	gameOverGuide_ = std::make_unique<Object3d>();
	gameOverGuide_->Initialize("GameOverGuide.obj");
	
	// skydome
	skydome_ = std::make_unique<Object3d>();
	skydome_->Initialize("skydome.obj");
	skydome_->SetPointLight(0.0f);
	skydomeTransform_.Initialize();

	// プレイヤーの見た目
	player_ = std::make_unique<Object3d>();
	player_->Initialize("Player.obj");

	playerTransform_.Initialize();
	playerTransform_.SetTranslate(GameOverDefaults::kPlayerInitTranslate);
	playerTransform_.SetRotate(GameOverDefaults::kPlayerInitRotate);
}

void GameOverScene::Update()
{
	// 入力
	input_->Update();

	// スペースでタイトルへ
	if (input_->TriggerKey(DIK_SPACE)) {
		returnToTitle_ = true;
	}
	// フェードイン開始
	if (returnToTitle_ && !fadeStarted_) {
		fadeManager_->FadeInStart(GameOverDefaults::kFadeStep, [this]() {
			SetSceneNo(TITLE);
		});
		fadeStarted_ = true;
	}
	
	// フェード
	fadeManager_->Update();

	// 3D更新
	gameOverText_->SetWorldTransform(gameOverTextTransform_);
	gameOverText_->Update();

	skydome_->SetWorldTransform(skydomeTransform_);
	skydome_->Update();

	gameOverGuide_->SetWorldTransform(gameOverGuideTransform_);
	gameOverGuide_->Update();

	// プレイヤー落下＆回転
	Vector3 t = playerTransform_.GetTranslate();
	t.y -= GameOverDefaults::kPlayerFallSpeed;
	playerTransform_.SetTranslate(t);

	Vector3 r = playerTransform_.GetRotate();
	r.x += GameOverDefaults::kPlayerRotateSpeedX;
	playerTransform_.SetRotate(r);
	
	player_->SetWorldTransform(playerTransform_);
	player_->Update();

	// カメラ
	cameraManager_->Update();
	Object3dCommon::GetInstance()->SetDefaultCamera(cameraManager_->GetMainCamera());
}

void GameOverScene::Draw()
{
	/*------3Dオブジェクト------*/
	Object3dCommon::GetInstance()->DrawSettings();
	gameOverText_->Draw();
	gameOverGuide_->Draw();
	player_->Draw();
	skydome_->Draw();

	/*------スプライト------*/
	SpriteCommon::GetInstance()->DrawSettings();
	fadeManager_->Draw();
}

void GameOverScene::Finalize()
{
}

void GameOverScene::DrawImGui()
{
#ifdef USE_IMGUI
	ImGui::Begin("GameOverScene");
	// 直接編集は既存のUI維持（内部はWorldTransformゲッターへ書き換え可）
	ImGui::DragFloat3("text_translate_", const_cast<float*>(&gameOverTextTransform_.GetTranslate().x));
	ImGui::DragFloat3("text_rotate_",    const_cast<float*>(&gameOverTextTransform_.GetRotate().x));
	ImGui::DragFloat3("guide_translate_", const_cast<float*>(&gameOverGuideTransform_.GetTranslate().x));
	ImGui::DragFloat3("guide_rotate_",    const_cast<float*>(&gameOverGuideTransform_.GetRotate().x));
	ImGui::DragFloat3("guide_scale_",     const_cast<float*>(&gameOverGuideTransform_.GetScale().x));
	ImGui::DragFloat3("player_translate_", const_cast<float*>(&playerTransform_.GetTranslate().x));
	ImGui::DragFloat3("player_rotate_",    const_cast<float*>(&playerTransform_.GetRotate().x));
	ImGui::End();
	cameraManager_->DrawImGui();
#endif
}

#include "TitleScene.h"
#include <imgui.h>
#include <Object3dCommon.h>

void TitleScene::Initialize(DirectXCommon* directXCommon, WinApp* winApp)
{
	// カメラマネージャの初期化
	cameraManager_ = std::make_unique<CameraManager>();
	cameraManager_->Initialize(Object3dCommon::GetInstance()->GetDefaultCamera());

	cameraManager_->SetCameraPosition(TitleDefaults::kCamPos);
	cameraManager_->SetCameraRotation(TitleDefaults::kCamRot);
	
	// 入力の初期化
	input_ = std::make_unique<Input>();
	input_->Initialize(winApp);

	// オーディオの初期化
	Audio::GetInstance()->Initialize();
	soundData1_ = Audio::GetInstance()->SoundLoadWave("resources/Alarm01.wav");
	
	// プレイヤーの初期化
	player_ = std::make_unique<Player>();
	player_->Initialize();

	playerTransform_.SetTranslate(TitleDefaults::kPlayerInitPos);

	// タイトルロゴの初期化
	titleLogo_ = std::make_unique<Object3d>();
	titleLogo_->Initialize("title.obj");

	// ワールド変換の初期化
	titleLogoTransform_.Initialize();
	titleLogoTransform_.SetRotate(TitleDefaults::kTitleLogoRot);
	titleLogoTransform_.SetTranslate(TitleDefaults::kTitleLogoPos);

	// skydomeの初期化
	skydome_ = std::make_unique<Object3d>();
	skydome_->Initialize("skydome.obj");
	skydome_->SetPointLight(0.0f);
	skydomeTransform_.Initialize();

	// フェードマネージャの初期化
	fadeManager_ = std::make_unique<FadeManager>();
	fadeManager_->Initialize();

	// フェード開始
	fadeManager_->FadeOutStart(TitleDefaults::kFadeStep);

	// ガイドオブジェクトの初期化
	titleGuide_ = std::make_unique<Object3d>();
	titleGuide_->Initialize("titleGuide.obj");

	PostEffectManager::GetInstance()->SetEffectEnabled(1, false);
}

void TitleScene::Update()
{
	// 入力の更新
	input_->Update();

	// スペースーキーでゲームシーンに切り替える
	if (input_->TriggerKey(DIK_SPACE))
	{
		// フェードイン開始
		isGameStart_ = true;
		fadeManager_->FadeInStart(TitleDefaults::kFadeStep);
	}
	// フェードが完了したらシーン切り替え
	if (fadeManager_->GetFadeState() == FadeManager::EffectState::Finish && isGameStart_)
	{
		SetSceneNo(GAMEPLAY);
	}

	// オブジェクトの更新
	// プレイヤー
	player_->SetPosition(playerTransform_.GetTranslate());
	player_->Update();
	
	// タイトルロゴ
	titleLogo_->SetWorldTransform(titleLogoTransform_);
	titleLogo_->Update();

	// skydome
	skydome_->SetWorldTransform(skydomeTransform_);
	skydome_->Update();

	// ガイドオブジェクト
	titleGuide_->Update();
	titleGuide_->SetTranslate(titleGuidePosition_);
	titleGuide_->SetRotate(titleGuideRotate_);
	titleGuide_->SetScale(titleGuideScale_);

	// フェードマネージャの更新
	fadeManager_->Update();

	// カメラの更新
	CameraMove();

	// カメラマネージャの更新
	cameraManager_->Update();
	Object3dCommon::GetInstance()->SetDefaultCamera(cameraManager_->GetMainCamera());
}

void TitleScene::Draw()
{
	/*------オブジェクトの描画------*/
	Object3dCommon::GetInstance()->DrawSettings();

	// プレイヤーの描画
	player_->Draw();

	// タイトルロゴの描画
	titleLogo_->Draw();

	// skydomeの描画
	skydome_->Draw();

	// ガイドオブジェクトの描画
	titleGuide_->Draw();

	/*------スプライトの更新------*/
	SpriteCommon::GetInstance()->DrawSettings();
	
	// フェードマネージャの描画
	fadeManager_->Draw();
}

void TitleScene::Finalize()
{
	// サウンドデータ解放
	Audio::GetInstance()->SoundUnload(&soundData1_);
	Audio::GetInstance()->Finalize();
}

void TitleScene::DrawImGui()
{
	// ImGui描画
#ifdef USE_IMGUI
	ImGui::Begin("TitleScene");
	// プレイヤーの位置
	ImGui::DragFloat3("playerTranslate", &playerTransform_.translate_.x);
	ImGui::DragFloat3("titlePosition", &titleLogoTransform_.translate_.x);
	ImGui::SliderFloat3("titleRotate", &titleLogoTransform_.rotate_.x, -3.14f, 3.14f);
	ImGui::SliderFloat3("titleScale", &titleLogoTransform_.scale_.x, 0.0f, 10.0f);
	// ガイドの位置
	ImGui::DragFloat3("titleGuidePosition", &titleGuidePosition_.x);
	ImGui::SliderFloat3("titleGuideRotate", &titleGuideRotate_.x, -3.14f, 3.14f);
	ImGui::SliderFloat3("titleGuideScale", &titleGuideScale_.x, 0.0f, 10.0f);

	// その他のImGui描画
	skydome_->DrawImGui();
	fadeManager_->DrawImGui();
	cameraManager_->DrawImGui();
	ImGui::End();
#endif
}

void TitleScene::CameraMove()
{
	// プレイヤーの位置を取得
	Vector3 playerPos = player_->GetCenterPosition();

	static float sTheta_ = 0.0f;
	sTheta_ += TitleDefaults::kCameraRotSpeed; // 回転速度（ラジアン）

	// カメラの位置を計算
	Vector3 cameraPos = {
		playerPos.x + std::cos(sTheta_) * TitleDefaults::kCameraRadius,
		TitleDefaults::kCameraHeight,
		playerPos.z + std::sin(sTheta_) * TitleDefaults::kCameraRadius
	};
	// カメラの向きを計算
	Vector3 toPlayer = playerPos - cameraPos;
	float yaw = std::atan2(toPlayer.z, toPlayer.x); // Y軸回転
	
	// カメラの位置と回転を設定
	Vector3 cameraRotate = { 0, -yaw + TitleDefaults::kPiOver2, 0.0f }; // 必要に応じて符号やオフセット調整
	player_->SetRotation(cameraRotate);
	skydomeTransform_.rotate_.y = cameraRotate.y;
	titleLogoTransform_.rotate_.y = camera_->GetRotate().y;
}


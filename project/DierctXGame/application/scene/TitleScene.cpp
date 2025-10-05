#include "TitleScene.h"
#include <imgui.h>
#include <Object3dCommon.h>

void TitleScene::Initialize(DirectXCommon* directXCommon, WinApp* winApp)
{
	sprite = std::make_unique<Sprite>();
	sprite->Initialize(directXCommon, "resources/mori_Red.png");

	//Object3dCommon::GetInstance()->SetDefaultCamera(camera_.get());
	
	input = std::make_unique<Input>();
	input->Initialize(winApp);
	Audio::GetInstance()->Initialize();
	soundData1 = Audio::GetInstance()->SoundLoadWave("resources/Alarm01.wav");
	// 音声再生
	//Audio::GetInstance()->SoundPlayWave(soundData1);

	player_ = std::make_unique<Player>();
	player_->Initialize();

	playerTransform_.translate_ = { 0.0f,-0.5f,-4.0f };

	titleLogo_ = std::make_unique<Object3d>();
	titleLogo_->Initialize("title.obj");

	titleLogoTransform_.Initialize();

	titleLogoTransform_.rotate_ = { -1.73f, 0.0f, 0.0f };
	titleLogoTransform_.translate_ = { 0.0f, 1.2f, 0.0f };

	skydome_ = std::make_unique<Object3d>();
	skydome_->Initialize("skydome.obj");
	skydome_->SetPointLight(0.0f);
	skydomeTransform_.Initialize();
}

void TitleScene::Update()
{
	// 入力の更新
	input->Update();

	// エンターキーでゲームシーンに切り替える
	if (input->TriggerKey(DIK_RETURN))
	{
		SetSceneNo(GAMEPLAY);
	}

	player_->SetPosition(playerTransform_.translate_);
	player_->Update();
	

	sprite->Update();
	sprite->SetPosition(spritePosition);

	titleLogo_->SetWorldTransform(titleLogoTransform_);
	titleLogo_->Update();

	skydome_->SetWorldTransform(skydomeTransform_);
	skydome_->Update();

	CameraMove();
}

void TitleScene::Draw()
{
	/*------スプライトの更新------*/
	SpriteCommon::GetInstance()->DrawSettings();
	//sprite->Draw();
	/*------オブジェクトの描画------*/
	Object3dCommon::GetInstance()->DrawSettings();
	player_->Draw();

	titleLogo_->Draw();

	skydome_->Draw();
}

void TitleScene::Finalize()
{
	Audio::GetInstance()->SoundUnload(&soundData1);
	Audio::GetInstance()->Finalize();
}

void TitleScene::DrawImGui()
{
	ImGui::Begin("TitleScene");
	ImGui::DragFloat3("playerTranslate", &playerTransform_.translate_.x);
	ImGui::DragFloat3("titlePosition", &titleLogoTransform_.translate_.x);
	ImGui::SliderFloat3("titleRotate", &titleLogoTransform_.rotate_.x, -3.14f, 3.14f);
	ImGui::SliderFloat3("titleScale", &titleLogoTransform_.scale_.x, 0.0f, 10.0f);
	skydome_->DrawImGui();
	ImGui::End();
}

void TitleScene::CameraMove()
{
	Vector3 playerPos = player_->GetCenterPosition();

	static float theta = 0.0f;
	theta += 0.005f; // 回転速度（ラジアン）

	float radius = 5.0f; // プレイヤーからの距離
	float height = 2.0f; // カメラの高さ

	Vector3 cameraPos = {
		playerPos.x + std::cos(theta) * radius,
		height,
		playerPos.z + std::sin(theta) * radius
	};

	Vector3 toPlayer = playerPos - cameraPos;
	float yaw = std::atan2(toPlayer.z, toPlayer.x); // Y軸回転
	
	Vector3 cameraRotate = { 0, -yaw + 3.14159f / 2.0f, 0.0f }; // 必要に応じて符号やオフセット調整
	player_->SetRotation(cameraRotate);
	skydomeTransform_.rotate_.y = cameraRotate.y;
	//camera_->SetTranslate(cameraPos);
	//camera_->SetRotate(cameraRotate);
	camera_->Update();
	titleLogoTransform_.rotate_.y = camera_->GetRotate().y;
	//Object3dCommon::GetInstance()->SetDefaultCamera(camera_.get());
}


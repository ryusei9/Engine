#include "TitleScene.h"
#include <imgui.h>
#include <Object3dCommon.h>
#include <Lerp.h>

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
	player_ = std::make_unique<Object3d>();
	player_->Initialize("player.obj");

	playerTransform_.SetTranslate(TitleDefaults::kPlayerInitPos);

	// タイトルロゴの初期化
	titleLogo_ = std::make_unique<Object3d>();
	titleLogo_->Initialize("title.obj");

	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(directXCommon, "resources/StageClear.png");

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

	// ドックの初期化
	dock_ = std::make_unique<Object3d>();
	dock_->Initialize("Dock.obj");
	dock_->SetPointLight(0.0f);

	dockTransform_.Initialize();
	dockTransform_.SetTranslate(TitleDefaults::kDockPos);
	

	PostEffectManager::GetInstance()->SetEffectEnabled(1, false);
}

void TitleScene::Update()
{
	input_->Update();

	if (input_->TriggerKey(DIK_SPACE) && startState_ == TitleStartState::None)
	{
		startState_ = TitleStartState::CameraMove;
		startTimer_ = 0.0f;

		startPlayerBasePos_ = player_->GetTranslate();
		startDockBasePos_ = dockTransform_.GetTranslate();
		startCameraPos_ = cameraManager_->GetMainCamera()->GetTranslate();

		// ★ 移動先をここで決める
		startPlayerTargetPos_ = {
			startPlayerBasePos_.x,
			startPlayerBasePos_.y,
			startPlayerBasePos_.z - 1.5f  // 少し後ろへ
		};

		startDockTargetPos_ = {
			startDockBasePos_.x,
			startDockBasePos_.y,
			startDockBasePos_.z - 1.5f
		};

		Vector3 rot = player_->GetRotate();
		rot.y = 4.7f;

		startRotY_ = rot.y;
		
	}

	if (startState_ != TitleStartState::None)
	{
		startTimer_ += 1.0f / 60.0f;

		switch (startState_)
		{
		case TitleStartState::CameraMove:
		{
			float t = std::clamp(startTimer_ / 1.0f, 0.0f, 3.0f);
			float e = EaseOut(t);

			Vector3 targetPos = {
				startPlayerBasePos_.x,
				startPlayerBasePos_.y + 1.5f,
				startPlayerBasePos_.z - 7.0f
			};

			float targetRotY = 4.7f;
			float newRotY = Lerp(startRotY_, targetRotY, e);

			cameraManager_->SetCameraPosition(Lerp(startCameraPos_, targetPos, e));

			// --- プレイヤー位置補間 ---
			Vector3 newPlayerPos = Lerp(startPlayerBasePos_, startPlayerTargetPos_, e);
			player_->SetTranslate(newPlayerPos);
			player_->SetRotate({ 0.0f, newRotY, 0.0f });

			// --- ドック位置補間 ---
			Vector3 newDockPos = Lerp(startDockBasePos_, startDockTargetPos_, e);
			dockTransform_.SetTranslate(newDockPos);
			dockTransform_.SetRotate({ 0.0f, newRotY, 0.0f });
			


			if (t >= 1.0f) {
				startState_ = TitleStartState::FloatUp;
				startTimer_ = 0.0f;
			}
		}
		break;

		case TitleStartState::FloatUp:
		{
			float t = std::clamp(startTimer_ / 1.0f, 0.0f, 1.0f);
			float e = EaseOut(t);

			Vector3 pos = startPlayerBasePos_;
			pos.y += 0.2f * e;
			player_->SetTranslate(pos);

			if (t >= 1.0f) {
				startState_ = TitleStartState::FlyAway;
				startTimer_ = 0.0f;
			}
		}
		break;

		case TitleStartState::FlyAway:
		{
			float t = std::clamp(startTimer_ / 0.8f, 0.0f, 1.0f);
			float e = EaseOut(t);

			Vector3 pos = player_->GetTranslate();
			pos.z += 1.0f * e;
			player_->SetTranslate(pos);

			if (t >= 1.0f) {
				fadeManager_->FadeInStart(TitleDefaults::kFadeStep);
				startState_ = TitleStartState::FadeOut;
			}
		}
		break;

		case TitleStartState::FadeOut:
			if (fadeManager_->GetFadeState() == FadeManager::EffectState::Finish)
			{
				RequestSceneChange(GAMEPLAY);
			}
			break;
		}
	}
	else
	{
		CameraMove();
		player_->SetTranslate(playerTransform_.GetTranslate());
	}
	
	player_->Update();
	titleLogo_->SetWorldTransform(titleLogoTransform_);
	titleLogo_->Update();
	// ガイドオブジェクト
	titleGuide_->Update();
	titleGuide_->SetTranslate(titleGuidePosition_);
	titleGuide_->SetRotate(titleGuideRotate_);
	titleGuide_->SetScale(titleGuideScale_);
	skydome_->SetWorldTransform(skydomeTransform_);
	skydome_->Update();
	dock_->SetWorldTransform(dockTransform_);
	dock_->Update();
	sprite_->SetPosition(spritePosition_);
	sprite_->Update();

	fadeManager_->Update();
	cameraManager_->Update();
	Object3dCommon::GetInstance()->SetDefaultCamera(cameraManager_->GetMainCamera());
}

void TitleScene::Draw()
{
	/*------オブジェクトの描画------*/
	Object3dCommon::GetInstance()->DrawSettings();

	// プレイヤーの描画
	player_->Draw();

	// ドックの描画
	dock_->Draw();

	if (startState_ == TitleStartState::None) {
		// タイトルロゴの描画
		titleLogo_->Draw();
		// ガイドオブジェクトの描画
		titleGuide_->Draw();
	}
	// skydomeの描画
	skydome_->Draw();

	


	/*------スプライトの更新------*/
	SpriteCommon::GetInstance()->DrawSettings();
	//sprite_->Draw();
	
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
	// ステート
	ImGui::Text("State: %d", static_cast<int>(startState_));
	
	// プレイヤーの位置
	Vector3 playerPos = playerTransform_.GetTranslate();
	if (ImGui::DragFloat3("playerTranslate", &playerPos.x)) {
		playerTransform_.SetTranslate(playerPos);
	}

	// タイトルロゴのトランスフォーム
	Vector3 titlePos = titleLogoTransform_.GetTranslate();
	Vector3 titleRot = titleLogoTransform_.GetRotate();
	Vector3 titleScale = titleLogoTransform_.GetScale();

	if (ImGui::DragFloat3("titlePosition", &titlePos.x)) {
		titleLogoTransform_.SetTranslate(titlePos);
	}
	if (ImGui::SliderFloat3("titleRotate", &titleRot.x, -3.14f, 3.14f)) {
		titleLogoTransform_.SetRotate(titleRot);
	}
	if (ImGui::SliderFloat3("titleScale", &titleScale.x, 0.0f, 10.0f)) {
		titleLogoTransform_.SetScale(titleScale);
	}
	// ガイドの位置
	ImGui::DragFloat3("titleGuidePosition", &titleGuidePosition_.x);
	ImGui::SliderFloat3("titleGuideRotate", &titleGuideRotate_.x, -3.14f, 3.14f);
	ImGui::SliderFloat3("titleGuideScale", &titleGuideScale_.x, 0.0f, 10.0f);

	// ドックのトランスフォーム
	Vector3 dockPos = dockTransform_.GetTranslate();
	Vector3 dockRot = dockTransform_.GetRotate();
	Vector3 dockScale = dockTransform_.GetScale();
	if (ImGui::DragFloat3("dockPosition", &dockPos.x)) {
		dockTransform_.SetTranslate(dockPos);
	}
	if (ImGui::SliderFloat3("dockRotate", &dockRot.x, -3.14f, 3.14f)) {
		dockTransform_.SetRotate(dockRot);
	}
	if (ImGui::SliderFloat3("dockScale", &dockScale.x, 0.0f, 10.0f)) {
		dockTransform_.SetScale(dockScale);
	}

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
	Vector3 playerPos = player_->GetTranslate();

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
	player_->SetRotate(cameraRotate);
	dockTransform_.SetRotate(cameraRotate);
	
	// skydomeの回転を設定
	Vector3 skydomeRot = skydomeTransform_.GetRotate();
	skydomeRot.y = cameraRotate.y;
	skydomeTransform_.SetRotate(skydomeRot);

	// タイトルロゴの回転を設定
	Vector3 titleRot = titleLogoTransform_.GetRotate();
	titleRot.y = camera_->GetRotate().y;
	titleLogoTransform_.SetRotate(titleRot);
}


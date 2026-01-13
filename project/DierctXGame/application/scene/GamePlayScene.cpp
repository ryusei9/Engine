#include "GamePlayScene.h"
#include "SRFramework.h"
#include "Object3dCommon.h"
#include "PlayerChargeBullet.h"
#include "Vector3.h"
#include "Inverse.h"
#include <iostream>
#include <Multiply.h>
#include <cmath>
#undef min
#undef max

void GamePlayScene::Initialize(DirectXCommon* directXCommon, WinApp* winApp)
{
	// スプライトの初期化
	sprite_ = std::make_unique<Sprite>();

	sprite_->Initialize(directXCommon, "resources/BackToTitle.png");
	// 入力の初期化
	input_ = Input::GetInstance();

	// オーディオの初期化
	Audio::GetInstance()->Initialize();
	soundData1_ = Audio::GetInstance()->SoundLoadWave("resources/Alarm01.wav");

	// フェードマネージャの初期化
	fadeManager_ = std::make_unique<FadeManager>();
	fadeManager_->Initialize();

	// フェード開始
	fadeManager_->FadeOutStart(GamePlayDefaults::kDeltaTime60Hz * 1.2f); // 0.02f相当

	// タイトルに戻るテキストのトランスフォームの初期化
	textTitle_.Initialize();
	textTitle_.SetTranslate(Vector3{ 3.333f, 2.857f, 10.000f });
	textTitle_.SetRotate(Vector3{ -1.495f, 0.0f, 0.0f });

	// オブジェクト3Dの初期化
	backToTitle_ = std::make_unique<Object3d>();
	backToTitle_->Initialize("BackToTitle.obj");
	backToTitle_->SetWorldTransform(textTitle_);

	// プレイヤーの初期化
	player_ = std::make_unique<Player>();
	player_->Initialize();
	
	// レベルデータのロード
	levelData_ = JsonLoader::Load("test"); // "resources/level1.json"など
	LoadLevel(levelData_);

	// プレイヤー配置データからプレイヤーを配置
	if (!levelData_->players.empty()) {
		auto& playerData = levelData_->players[0];
		player_->SetPosition(playerData.translation);
		player_->SetRotation(playerData.rotation);
	}

	// プレイヤーの弾のリストを取得
	playerBullets_ = &player_->GetBullets();
	playerChargeBullets_ = &player_->GetChargeBullets();

	// スカイボックスの初期化
	skybox_ = std::make_unique<Skybox>();
	skybox_->Initialize("resources/rostock_laage_airport_4k.dds");

	// オブジェクト生成
	CreateObjectsFromLevelData();

	// 衝突マネージャの生成
	collisionManager_ = std::make_unique<CollisionManager>();
	collisionManager_->Initialize();

	// カメラマネージャーの初期化
	cameraManager_ = std::make_unique<CameraManager>();
	cameraManager_->Initialize(Object3dCommon::GetInstance()->GetDefaultCamera());

	// ゲームオーバー用タイマー初期化
	gameOverTimer_ = GamePlayDefaults::kGameOverTimerSec;

	// ゲームクリアテキストの初期化
	gameClearText_ = std::make_unique<Object3d>();
	gameClearText_->Initialize("StageClear.obj");
	
	// ゲームクリアテキストのトランスフォーム初期化
	gameClearTextTransform_.Initialize();
	gameClearTextTransform_.SetScale(Vector3(0.528f, 0.528f, 0.528f));
	gameClearTextTransform_.SetRotate(Vector3{ -1.694f, 0.0f, 0.0f });

	// スペースキーを押してくださいテキストの初期化
	pressSpaceKeyText_ = std::make_unique<Object3d>();
	pressSpaceKeyText_->Initialize("PressSpaceKey.obj");

	// スペースキーを押してくださいテキストのトランスフォーム初期化
	pressSpaceKeyTransform_.Initialize();
	pressSpaceKeyTransform_.SetScale(Vector3(0.4f, 0.4f, 0.4f));
	pressSpaceKeyTransform_.SetRotate(Vector3{ -1.694f, 0.0f, 0.0f });

	// カメラステート初期化: Intro_Pan から開始
	currentCameraState_ = std::make_unique<CameraStateIntroPan>();
	currentCameraState_->Enter(this);
}

void GamePlayScene::Update()
{
	// 入力の更新
	input_->Update();

	// Tキーでタイトルシーンに切り替える
	if (input_->TriggerKey(DIK_T))
	{
		SetSceneNo(TITLE);
	}

	// プレイヤーが死んでいるかチェック
	if (!player_->GetIsAlive()) {
		// ゲームオーバー処理開始
		isGameOver_ = true;
	}

	// フェード処理
	// ゲームオーバー時の更新
	if (isGameOver_ && !fadeStarted_) {
		gameOverTimer_ -= GamePlayDefaults::kDeltaTime60Hz; // 60FPS想定
		if (gameOverTimer_ <= 0.0f) {
			fadeManager_->FadeInStart(GamePlayDefaults::kDeltaTime60Hz * 1.2f, [this]() { // 0.02f相当
				SetSceneNo(GAMEOVER);
				});
			fadeStarted_ = true;
		}
	}
	// ゲームクリア時のフェード処理
	if (isEnd_ && !fadeStarted_) {
		gameOverTimer_ -= GamePlayDefaults::kDeltaTime60Hz; // 60FPS想定
		if (gameOverTimer_ <= 0.0f) {
			fadeManager_->FadeInStart(GamePlayDefaults::kDeltaTime60Hz * 1.2f, [this]() { // 0.02f相当
				SetSceneNo(TITLE);
				});
			fadeStarted_ = true;
		}
	}

	// カメラステートの更新
	if (currentCameraState_) {
		currentCameraState_->Update(this, GamePlayDefaults::kDeltaTime60Hz);
	}

	// ゲームオーバーじゃないとき
	if (!isGameOver_) {
		player_->Update();

		// プレイヤーの弾の奥行き調整
		for (auto& bullet : player_->GetBullets()) {
			if (bullet && bullet->IsAlive()) {
				Camera* cam = cameraManager_->GetMainCamera();
				Vector3 bulletPos = bullet->GetTranslate();
				bulletPos.z = cam->GetTranslate().z + 10.0f; // プレイヤーと同じ奥行
				bullet->SetTranslate(bulletPos);
			}
		}
	}

	// 読み込んだ全オブジェクトの更新
	for (auto& obj : objects_) {
		obj->Update();
	}

	// 敵の更新
	for (auto& enemy : enemies_) {
		enemy->SetPlayer(player_.get());
		enemy->Update();

		Camera* cam = cameraManager_->GetMainCamera();
		Vector3 enemyPos = enemy->GetPosition();
		bool inView = IsInCameraView(enemyPos);
		enemy->SetControlEnabled(inView);
		enemy->SetZ(cam->GetTranslate().z + 10.0f);

		for (auto& bullet : enemy->GetBullets()) {
			if (bullet && bullet->IsAlive()) {
				Vector3 bulletPos = bullet->GetPosition();
				bulletPos.z = cam->GetTranslate().z + 10.0f;
				bullet->SetPosition(bulletPos);
			}
		}
	}

	// ゲームクリアテキストの更新
	if (gameClearTextVisible_ && gameClearText_) {
		// カメラ位置を取得して Y を +1.2 する
		Camera* cam = cameraManager_->GetMainCamera();
		if (cam) {
			// テキストの位置調整
			Vector3 camPos = cam->GetTranslate();
			gameClearTextTransform_.translate_.x = camPos.x;
			gameClearTextTransform_.translate_.y = camPos.y + 0.6f;
			gameClearTextTransform_.translate_.z = camPos.z + 5.0f;

			pressSpaceKeyTransform_.translate_ = camPos + Vector3(0.0f, -0.6f, 5.0f);

			gameClearText_->SetWorldTransform(gameClearTextTransform_);
			pressSpaceKeyText_->SetWorldTransform(pressSpaceKeyTransform_);

			gameClearText_->Update();
			pressSpaceKeyText_->Update();
		}
	}

	// skyboxの更新
	skybox_->Update();

	// タイトルに戻るテキストの更新
	backToTitle_->SetWorldTransform(textTitle_);
	backToTitle_->Update();

	// フェードマネージャの更新
	fadeManager_->Update();

	// カメラの更新
	switch (cameraMode_) {
	case CameraMode::Free:
		// なにもしない
		break;
	case CameraMode::FollowPlayer:
		// プレイヤーについて行く
		cameraManager_->SetCameraPosition(player_->GetWorldTransform().GetTranslate() + Vector3{ 0.0f, 1.0f, -10.0f });
		cameraManager_->SetCameraRotation(Vector3{ 0.1f, 0.0f, 0.0f });
		break;
	case CameraMode::DynamicFollow:
		// プレイヤーを注視しつつ追従
		cameraManager_->MoveTargetAndCamera(player_->GetWorldTransform(), Vector3{ 0.0f, 1.0f, -10.0f });
		cameraManager_->LookAtTarget(player_->GetPosition());
		break;
	case CameraMode::CenterPlayer:
		// プレイヤーを中心に注視
		cameraManager_->LookAtTarget(player_->GetPosition(), true);
		break;
	}

	cameraManager_->Update();
	Object3dCommon::GetInstance()->SetDefaultCamera(cameraManager_->GetMainCamera());

	sprite_->SetPosition(spritePosition_);
	sprite_->Update();

	// 衝突マネージャの更新
	collisionManager_->Update();
	CheckAllCollisions();// 衝突判定と応答

	// 敵
	enemies_.erase(
		std::remove_if(enemies_.begin(), enemies_.end(),
			[&](const std::unique_ptr<Enemy>& e) {
				if (!e->IsAlive()) {
					collisionManager_->RemoveCollider(e.get());
					return true;
				}
				return false;
			}),
		enemies_.end()
	);

	// プレイヤー弾
	playerBullets_->erase(
		std::remove_if(playerBullets_->begin(), playerBullets_->end(),
			[](const std::unique_ptr<PlayerBullet>& b) {
				return !b->IsAlive();
			}),
		playerBullets_->end()
	);

	// チャージ弾
	playerChargeBullets_->erase(
		std::remove_if(playerChargeBullets_->begin(), playerChargeBullets_->end(),
			[](const std::unique_ptr<PlayerChargeBullet>& b) {
				return !b->IsAlive();
			}),
		playerChargeBullets_->end()
	);
}

void GamePlayScene::Draw()
{
	/*------UIの描画------*/
	SpriteCommon::GetInstance()->DrawSettings();

	/*------オブジェクトの描画------*/
	Object3dCommon::GetInstance()->DrawSettings();

	// 読み込んだ全オブジェクトの描画
	for (auto& obj : objects_) {
		obj->Draw();
	}

	if (!isGameOver_) {
		// プレイヤーの描画
		player_->Draw();
	}

	// 敵の描画
	for (auto& enemy : enemies_) {
		enemy->Draw();
	}

	// クリアテキストの描画
	if (gameClearTextVisible_ && gameClearText_) {
		gameClearText_->Draw();
		pressSpaceKeyText_->Draw();
	}

	/*------skyboxの描画------*/
	skybox_->DrawSettings();
	skybox_->Draw();

	/*------スプライトの更新------*/
	SpriteCommon::GetInstance()->DrawSettings();

	// フェードの描画
	fadeManager_->Draw();
}

void GamePlayScene::Finalize()
{
	// オーディオの終了処理
	Audio::GetInstance()->SoundUnload(&soundData1_);
	Audio::GetInstance()->Finalize();
}

void GamePlayScene::DrawImGui()
{
	// ImGuiウィンドウの表示
#ifdef USE_IMGUI
	ImGui::Begin("GamePlayScene");
	ImGui::Text("SPACE : Shot Bullet");
	ImGui::Text("WASD : Move Player");
	// パーティクルエミッター1の位置
	ImGui::SliderFloat3("sprite Position", &spritePosition_.x, 1.0f, 50.0f);
	// CameraMode選択用Combo
	static const char* cameraModeItems[] = { "Free", "FollowPlayer", "DynamicFollow", "CenterPlayer" };
	int32_t cameraModeIndex = static_cast<int32_t>(cameraMode_);
	if (ImGui::Combo("Camera Mode", &cameraModeIndex, cameraModeItems, IM_ARRAYSIZE(cameraModeItems))) {
		cameraMode_ = static_cast<CameraMode>(cameraModeIndex);
	}
	ImGui::SliderFloat3("text Position", &textTitle_.translate_.x, -10.0f, 10.0f);
	ImGui::SliderFloat3("text rotation", &textTitle_.rotate_.x, -3.14f, 3.14f);

	ImGui::SliderFloat3("clear Text Position", &gameClearTextTransform_.translate_.x, -10.0f, 10.0f);
	ImGui::SliderFloat3("clear Text rotation", &gameClearTextTransform_.rotate_.x, -3.14f, 3.14f);
	ImGui::SliderFloat3("clear Text Scale", &gameClearTextTransform_.scale_.x, 0.1f, 10.0f);
	// レベルデータから生成したオブジェクトのImGui調整
	DrawImGuiImportObjectsFromJson();
	ImGui::End();
	player_->DrawImGui();

	for (auto& enemy : enemies_) {
		enemy->DrawImGui();
	}

	skybox_->DrawImGui();
	cameraManager_->DrawImGui();
	fadeManager_->DrawImGui();
#endif
}

// ステート遷移関数
void GamePlayScene::ChangeState(std::unique_ptr<CameraState> newState)
{
	if (currentCameraState_) {
		currentCameraState_->Exit(this);
	}
	currentCameraState_ = std::move(newState);
	currentCameraState_->Enter(this);
}

void GamePlayScene::TransitionToGameplayState()
{
	ChangeState(std::make_unique<CameraStateGameplay>());
}

void GamePlayScene::TransitionToClearHoldState()
{
	ChangeState(std::make_unique<CameraStateClearHold>());
}

void GamePlayScene::TransitionToClearZoomState()
{
	ChangeState(std::make_unique<CameraStateClearZoom>());
}

void GamePlayScene::TransitionToClearFlyAwayState()
{
	ChangeState(std::make_unique<CameraStateClearFlyAway>());
}

void GamePlayScene::OnGameClear()
{
	if (!isGameClear_) {
		isGameClear_ = true;
		TransitionToClearHoldState();
	}
}

void GamePlayScene::ShowGameClearText()
{
	gameClearTextVisible_ = true;
	if (gameClearText_) {
		Camera* cam = cameraManager_->GetMainCamera();
		if (cam) {
			Vector3 camPos = cam->GetTranslate();
			gameClearText_->SetTranslate(camPos + Vector3{ 0.0f, 1.2f, 0.0f });
			gameClearText_->Update();
		}
	}
}

void GamePlayScene::HideGameClearText()
{
	gameClearTextVisible_ = false;
}

void GamePlayScene::StartGameClearFade()
{
	gameClearFadeStarted_ = true;
	fadeManager_->FadeInStart(GamePlayDefaults::kDeltaTime60Hz * 1.2f, [this]() {
		SetSceneNo(TITLE);
	});
}

void GamePlayScene::CreateObjectsFromLevelData()
{
	for (auto& objectData : levelData_->objects) {
		if (objectData.disabled) continue;
		Model* model = nullptr;
		auto it = models_.find(objectData.fileName);
		if (it != models_.end()) { model = it->second.get(); }
		// モデルを指定して3Dオブジェクトを生成
		auto newObject = std::make_unique<Object3d>();
		newObject->Initialize(objectData.fileName + ".obj");
		// 平行移動
		newObject->SetTranslate(objectData.translation);
		// 回転角
		newObject->SetRotate(objectData.rotation);
		// スケーリング
		newObject->SetScale(objectData.scaling);
		objects_.push_back(std::move(newObject));
	}

	// レベルデータから敵を生成、配置
	for (auto& enemyData : levelData_->enemies) {
		// ファイル名から登録済みモデルを検索
		Model* model = nullptr;
		auto it = models_.find(enemyData.fileName);
		if (it != models_.end()) { model = it->second.get(); }
		// 敵オブジェクトの生成
		auto newEnemy = std::make_unique<Enemy>();
		newEnemy->Initialize();
		newEnemy->SetPosition(enemyData.translation);
		newEnemy->SetRotation(enemyData.rotation);
		newEnemy->SetPlayer(player_.get());
		enemies_.push_back(std::move(newEnemy));
	}
}

void GamePlayScene::DrawImGuiImportObjectsFromJson()
{
#ifdef USE_IMGUI
	// レベルデータから生成したオブジェクトのImGui調整
	for (size_t i = 0; i < objects_.size(); ++i) {
		auto& obj = objects_[i];
		ImGui::PushID(static_cast<int32_t>(i)); // 複数オブジェクト対応

		// 位置・回転・スケールの取得
		Vector3 pos = obj->GetTranslate();
		Vector3 rot = obj->GetRotate();
		Vector3 scale = obj->GetScale();

		if (ImGui::SliderFloat3("position", &pos.x, -10.0f, 10.0f)) {
			obj->SetTranslate(pos);
		}
		if (ImGui::SliderFloat3("Rotation", &rot.x, -180.0f, 180.0f)) {
			obj->SetRotate(rot);
		}
		if (ImGui::SliderFloat3("Scale", &scale.x, 0.01f, 10.0f)) {
			obj->SetScale(scale);
		}

		ImGui::PopID();
	}
	// レベルデータから生成した敵のImGui調整
	for (size_t i = 0; i < enemies_.size(); ++i) {
		auto& enemy = enemies_[i];
		ImGui::Begin("Enemy");
		ImGui::Text("Enemy %d", static_cast<int32_t>(i + 1));
		ImGui::PushID(static_cast<int32_t>(i)); // 複数オブジェクト対応

		// 位置・回転・スケールの取得
		Vector3 pos = enemy->GetPosition();
		Vector3 rot = enemy->GetRotation();
		Vector3 scale = enemy->GetScale();

		if (ImGui::SliderFloat3("position", &pos.x, -10.0f, 10.0f)) {
			enemy->SetPosition(pos);
		}
		if (ImGui::SliderFloat3("Rotation", &rot.x, -180.0f, 180.0f)) {
			enemy->SetRotation(rot);
		}
		if (ImGui::SliderFloat3("Scale", &scale.x, 0.01f, 10.0f)) {
			enemy->SetScale(scale);
		}

		ImGui::PopID();
		ImGui::End();
	}
#endif
}

void GamePlayScene::CheckAllCollisions()
{
	// 衝突マネージャのリセット
	collisionManager_->Reset();

	// コライダーをリストに登録
	collisionManager_->AddCollider(player_.get());

	// 敵とその弾を登録
	for (auto& enemy : enemies_) {
		if (enemy && enemy->GetState() == Enemy::EnemyState::Alive) {
			collisionManager_->AddCollider(enemy.get());

			// 敵の弾も登録
			for (const auto& bullet : enemy->GetBullets()) {
				if (bullet) {
					collisionManager_->AddCollider(bullet.get());
				}
			}
		}
	}

	// プレイヤー弾
	for (const auto& bullet : *playerBullets_) {
		if (bullet && bullet->IsAlive()) {
			collisionManager_->AddCollider(bullet.get());
		}
	}

	// チャージ弾
	for (const auto& chargeBullet : *playerChargeBullets_) {
		if (chargeBullet && chargeBullet->IsAlive()) {
			collisionManager_->AddCollider(chargeBullet.get());
		}
	}

	// 衝突判定と応答
	collisionManager_->CheckCollision();
}

void GamePlayScene::LoadLevel(const LevelData* levelData)
{
	curvePoints_.clear();
	if (!levelData->curves.empty()) {
		curvePoints_ = levelData->curves[0].points;
	}
}

void GamePlayScene::UpdateGameClear()
{
	// この関数は削除可能（ステートマシンが処理を引き継ぎ）
}

void GamePlayScene::RestrictPlayerInsideCameraView()
{
	Camera* cam = cameraManager_->GetMainCamera();
	Matrix4x4 viewProj = cam->GetViewProjectionMatrix();
	Vector3 playerPos = player_->GetPosition();
	Vector4 clipPos = Multiply::Multiply(viewProj, Vector4{ playerPos.x, playerPos.y, playerPos.z, 1.0f });
	Vector3 ndcPos = { clipPos.x / clipPos.w, clipPos.y / clipPos.w, clipPos.z / clipPos.w };
	ndcPos.x = std::clamp(ndcPos.x, -0.9f, 0.9f);
	ndcPos.y = std::clamp(ndcPos.y, -0.9f, 0.9f);
	Vector4 newClipPos = { ndcPos.x * clipPos.w, ndcPos.y * clipPos.w, ndcPos.z * clipPos.w, clipPos.w };
	Matrix4x4 invViewProj = Inverse::Inverse(viewProj);
	Vector4 newWorldPos = Multiply::Multiply(invViewProj, newClipPos);
	Vector3 clampedWorldPos = { newWorldPos.x / newWorldPos.w, newWorldPos.y / newWorldPos.w, newWorldPos.z / newWorldPos.w };
	player_->SetPosition(clampedWorldPos);
}

bool GamePlayScene::IsInCameraView(const Vector3& worldPos)
{
	Camera* cam = cameraManager_->GetMainCamera();
	Matrix4x4 viewProj = cam->GetViewProjectionMatrix();
	Vector4 clip = Multiply::Multiply(viewProj, Vector4{ worldPos.x, worldPos.y, worldPos.z, 1.0f });
	if (clip.w == 0.0f) return false;
	Vector3 ndc = { clip.x / clip.w, clip.y / clip.w, clip.z / clip.w };
	return (ndc.x >= -1.0f && ndc.x <= 1.0f && ndc.y >= -1.0f && ndc.y <= 1.0f && ndc.z >= 0.0f && ndc.z <= 1.0f);
}
#include "GamePlayScene.h"
#include "SRFramework.h"
#include "Object3dCommon.h"
#include "PlayerChargeBullet.h"
void GamePlayScene::Initialize(DirectXCommon* directXCommon, WinApp* winApp)
{
	sprite = std::make_unique<Sprite>();
	input = Input::GetInstance();
	// テクスチャ"モリ"を使用
	sprite->Initialize(directXCommon, "resources/BackToTitle.png");

	Audio::GetInstance()->Initialize();
	soundData1 = Audio::GetInstance()->SoundLoadWave("resources/Alarm01.wav");
	// 音声再生
	//Audio::GetInstance()->SoundPlayWave(soundData1);

	// パーティクルマネージャの初期化
	particleManager = ParticleManager::GetInstance();
	//particleManager->GetInstance()->SetParticleType(ParticleType::Cylinder);

	// テクスチャ"モリ"を使用
	particleManager->GetInstance()->CreateParticleGroup("mori", "resources/gradationLine.png");
	// テクスチャ"UV"を使用
	particleManager->GetInstance()->CreateParticleGroup("uv", "resources/uvChecker.png");

	particleEmitter1 = std::make_unique<ParticleEmitter>(particleManager, "mori");


	//particleEmitter2 = std::make_unique<ParticleEmitter>(particleManager, "uv");

	//particleEmitter2->SetUseRingParticle(false);

	fadeManager_ = std::make_unique<FadeManager>();
	fadeManager_->Initialize();

	fadeManager_->FadeOutStart(0.02f);

	// ボールの初期化
	ball = std::make_unique<Object3d>();
	ball->Initialize("monsterBall.obj");

	ballTransform.Initialize();
	ball->SetSkyboxFilePath("resources/skybox.dds");
	ballTransform.translate_ = { 0.0f,0.0f,5.0f };  // 座標

	ball->SetWorldTransform(ballTransform);

	// ボールの初期化
	ground = std::make_unique<Object3d>();
	ground->Initialize("terrain.obj");

	groundTransform.Initialize();
	groundTransform.translate_ = { 0.0f,0.0f,5.0f }; // 座標
	ground->SetWorldTransform(groundTransform);

	textTitle.Initialize();
	textTitle.translate_ = { 3.333f,2.857f,10.000f };
	textTitle.rotate_ = { -1.495f,0.0f,0.0f };

	BackToTitle = std::make_unique<Object3d>();
	BackToTitle->Initialize("BackToTitle.obj");
	BackToTitle->SetWorldTransform(textTitle);

	// プレイヤーの初期化
	player_ = std::make_unique<Player>();
	player_->Initialize();
	//player_->SetBullet(playerBullet_.get());
	// レベルデータのロード
	levelData_ = JsonLoader::Load("test"); // "resources/level1.json"など
	LoadLevel(levelData_);

	// プレイヤー配置データからプレイヤーを配置
	if (!levelData_->players.empty()) {
		auto& playerData = levelData_->players[0];
		player_->SetPosition(playerData.translation);
		player_->SetRotation(playerData.rotation);
	}

	playerBullets_ = &player_->GetBullets();
	playerChargeBullets_ = &player_->GetChargeBullets();

	skybox_ = std::make_unique<Skybox>();
	skybox_->Initialize("resources/rostock_laage_airport_4k.dds");

	// オブジェクト生成
	CreateObjectsFromLevelData();

	// 衝突マネージャの生成
	collisionManager_ = std::make_unique<CollisionManager>();
	collisionManager_->Initialize();

	cameraManager_ = std::make_unique<CameraManager>();
	cameraManager_->Initialize(Object3dCommon::GetInstance()->GetDefaultCamera());
	
	// スタート演出用カメラ初期化
	isStartCameraEasing_ = true;
	startCameraTimer_ = 0.0f;
	cameraManager_->SetCameraPosition(startCameraPos_);
	cameraManager_->SetCameraRotation(startCameraRot_);

	gameOverTimer_ = 2.0f;
	
#ifdef _DEBUG
	isStartCameraEasing_ = false; // デバッグ時はスタート演出をスキップ
	cameraMode_ = CameraMode::Free;
#endif
}

void GamePlayScene::Update()
{
	// 入力の更新
	input->Update();

	// Tキーでタイトルシーンに切り替える
	if (input->TriggerKey(DIK_T))
	{
		SetSceneNo(TITLE);
	}

	if (!player_->GetIsAlive()) {
		isGameOver_ = true;
	}

	if (isGameOver_ && !fadeStarted_) {
		gameOverTimer_ -= 1.0f / 60.0f; // 60FPS想定
		if (gameOverTimer_ <= 0.0f) {
			fadeManager_->FadeInStart(0.02f, [this]() {
				SetSceneNo(GAMEOVER);
				});
			fadeStarted_ = true;
		}
	}

	// ゲームオーバーじゃないとき
	if (!isGameOver_) {
		// プレイヤーの更新
		player_->Update();
	}


	//enemy_->SetPlayer(player_.get());
	// 敵の更新
	//enemy_->Update();

	// 読み込んだ全オブジェクトの更新
	for (auto& obj : objects) {
		obj->Update();
	}

	for (auto& enemy : enemies_) {
		enemy->SetPlayer(player_.get());
		enemy->Update();
	}

	// パーティクルグループ"モリ"の更新
	//particleEmitter1->SetPosition(particlePosition1);
	//particleEmitter1->SetParticleRate(1);
	//particleEmitter1->Update();

	// 衝突マネージャの更新
	collisionManager_->Update();
	CheckAllCollisions();// 衝突判定と応答
	//// パーティクルグループ"UV"の更新
	/*particleEmitter2->SetPosition(particlePosition2);
	particleEmitter2->SetParticleRate(8);
	particleEmitter2->Update();*/
	skybox_->Update();
	BackToTitle->SetWorldTransform(textTitle);
	BackToTitle->Update();
	fadeManager_->Update();


	// カメラの更新
	switch (cameraMode_) {
	case CameraMode::Free:
		// なにもしない
		break;
	case CameraMode::FollowPlayer:
		//cameraManager_->MoveTargetAndCamera(player_->GetWorldTransform(), Vector3{ 0.0f,1.0f,-10.0f });
		//cameraManager_->LookAtTarget(player_->GetPosition());
		cameraManager_->SetCameraPosition(player_->GetWorldTransform().translate_ + Vector3{ 0.0f,1.0f,-10.0f });
		cameraManager_->SetCameraRotation(Vector3{ 0.1f,0.0f,0.0f });
		break;
	case CameraMode::DynamicFollow:
		cameraManager_->MoveTargetAndCamera(player_->GetWorldTransform(), Vector3{ 0.0f,1.0f,-10.0f });
		cameraManager_->LookAtTarget(player_->GetPosition());
		break;
	}

	// スタート演出カメラ初期化
	if (isStartCameraEasing_) {
		player_->SetPlayerControlEnabled(false); // プレイヤー操作無効化
		UpdateStartCameraEasing();
		cameraManager_->Update();
		Object3dCommon::GetInstance()->SetDefaultCamera(cameraManager_->GetMainCamera());
		return; // 他の更新をスキップ（必要に応じて調整）
	} else {

		// プレイヤーをカーブに沿って移動
		UpdatePlayerOnCurve();
	}

	cameraManager_->Update();
	Object3dCommon::GetInstance()->SetDefaultCamera(cameraManager_->GetMainCamera());

	// スプライトの更新
	sprite->SetPosition(spritePosition);
	sprite->Update();

	/*------オブジェクトの更新------*/
	// ボールの更新
	//ball->Update();
	//ball->SetWorldTransform(ballTransform);
	//ground->Update();
	//ground->SetWorldTransform(groundTransform);
}

void GamePlayScene::Draw()
{
	/*------UIの描画------*/
	SpriteCommon::GetInstance()->DrawSettings();
	//sprite->Draw();

	/*------オブジェクトの描画------*/
	Object3dCommon::GetInstance()->DrawSettings();

	// 読み込んだ全オブジェクトの描画
	for (auto& obj : objects) {
		obj->Draw();
	}
	//ground->Draw();
	if (!isGameOver_) {
		// プレイヤーの描画
		player_->Draw();
	} else {

	}

	// 敵の描画
	//enemy_->Draw();
	for (auto& enemy : enemies_) {
		enemy->Draw();
	}
	// ボールの描画
	//ball->Draw();
	if (!isStartCameraEasing_) {
		//BackToTitle->Draw();
	}
	/*------skyboxの描画------*/
	skybox_->DrawSettings();
	skybox_->Draw();

	/*------スプライトの更新------*/
	SpriteCommon::GetInstance()->DrawSettings();

	fadeManager_->Draw();
}

void GamePlayScene::Finalize()
{
	Audio::GetInstance()->SoundUnload(&soundData1);
	Audio::GetInstance()->Finalize();
}

void GamePlayScene::DrawImGui()
{
	ImGui::Begin("GamePlayScene");
	ImGui::Text("SPACE : Shot Bullet");
	ImGui::Text("WASD : Move Player");
	// パーティクルエミッター1の位置
	ImGui::SliderFloat3("sprite Position", &spritePosition.x, 1.0f, 50.0f);
	// CameraMode選択用Combo
	static const char* cameraModeItems[] = { "Free", "FollowPlayer", "DynamicFollow" };
	int cameraModeIndex = static_cast<int>(cameraMode_);
	if (ImGui::Combo("Camera Mode", &cameraModeIndex, cameraModeItems, IM_ARRAYSIZE(cameraModeItems))) {
		cameraMode_ = static_cast<CameraMode>(cameraModeIndex);
	}
	ImGui::SliderFloat3("text Position", &textTitle.translate_.x, -10.0f, 10.0f);
	ImGui::SliderFloat3("text rotation", &textTitle.rotate_.x, -3.14f, 3.14f);
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
	//ball->DrawImGui();
}

void GamePlayScene::CreateObjectsFromLevelData()
{
	// レベルデータからオブジェクトを生成、配置
	for (auto& objectData : levelData_->objects) {
		if (objectData.disabled) {
			// 無効なオブジェクトはスキップ
			continue;
		}
		// ファイル名から登録済みモデルを検索
		Model* model = nullptr;
		auto it = models.find(objectData.fileName);
		if (it != models.end()) { model = it->second.get(); }
		// モデルを指定して3Dオブジェクトを生成
		auto newObject = std::make_unique<Object3d>();
		newObject->Initialize(objectData.fileName + ".obj");
		// 平行移動
		newObject->SetTranslate(objectData.translation);
		// 回転角
		newObject->SetRotate(objectData.rotation);
		// スケーリング
		newObject->SetScale(objectData.scaling);
		objects.push_back(std::move(newObject));
	}

	// レベルデータから敵を生成、配置
	for (auto& enemyData : levelData_->enemies) {

		// ファイル名から登録済みモデルを検索
		Model* model = nullptr;
		auto it = models.find(enemyData.fileName);
		if (it != models.end()) { model = it->second.get(); }
		// 敵オブジェクトの生成
		auto newEnemy = std::make_unique<Enemy>();
		newEnemy->Initialize();
		newEnemy->SetPosition(enemyData.translation);
		newEnemy->SetRotation(enemyData.rotation);
		newEnemy->SetPlayer(player_.get());
		enemyBullets_ = &newEnemy->GetBullets();
		enemies_.push_back(std::move(newEnemy));
	}
}

void GamePlayScene::DrawImGuiImportObjectsFromJson()
{
	// レベルデータから生成したオブジェクトのImGui調整
	for (size_t i = 0; i < objects.size(); ++i) {
		auto& obj = objects[i];
		ImGui::PushID(static_cast<int>(i)); // 複数オブジェクト対応

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
	for (size_t i = 0;i < enemies_.size();++i) {
		auto& enemy = enemies_[i];
		ImGui::Begin("Enemy");
		ImGui::Text("Enemy %d", static_cast<int>(i + 1));
		ImGui::PushID(static_cast<int>(i)); // 複数オブジェクト対応

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
}

void GamePlayScene::CheckAllCollisions()
{
	// 衝突マネージャのリセット
	collisionManager_->Reset();

	// コライダーをリストに登録
	collisionManager_->AddCollider(player_.get());

	for (auto& enemy : enemies_) {
		collisionManager_->AddCollider(enemy.get());
	}

	// 複数についてコライダーをリストに登録
	for (const auto& bullet : *playerBullets_)
	{
		collisionManager_->AddCollider(bullet.get());
	}
	for (const auto& chargeBullet : *playerChargeBullets_) {
		collisionManager_->AddCollider(chargeBullet.get());
	}
	// 敵の弾
	for (const auto& bullet : *enemyBullets_) {
		collisionManager_->AddCollider(bullet.get());
	}

	// 衝突判定と応答
	collisionManager_->CheckCollision();
}

void GamePlayScene::LoadLevel(const LevelData* levelData)
{
	// カーブ座標を保存
	curvePoints_.clear();
	if (!levelData->curves.empty()) {
		// 例: 最初のカーブのみ使用
		curvePoints_ = levelData->curves[0].points;
	}
	curveProgress_ = 0.0f;
	curveIndex_ = 0;
}

void GamePlayScene::UpdateStartCameraEasing()
{
	startCameraTimer_ += 1.0f / 60.0f; // フレームレート固定なら
	float t = std::clamp(startCameraTimer_ / startCameraDuration_, 0.0f, 1.0f);

	// イージング（easeInOutCubic）
	float easeT;
	if (t < 0.5f) {
		easeT = 4.0f * t * t * t;
	} else {
		float p = (t - 1.0f);
		easeT = 1.0f + 4.0f * p * p * p;
	}

	Vector3 pos = {
		std::lerp(startCameraPos_.x, endCameraPos_.x, easeT),
		std::lerp(startCameraPos_.y, endCameraPos_.y, easeT),
		std::lerp(startCameraPos_.z, endCameraPos_.z, easeT)
	};
	Vector3 rot = {
		std::lerp(startCameraRot_.x, endCameraRot_.x, easeT),
		std::lerp(startCameraRot_.y, endCameraRot_.y, easeT),
		std::lerp(startCameraRot_.z, endCameraRot_.z, easeT)
	};

	cameraManager_->SetCameraPosition(pos);
	//cameraManager_->SetCameraRotation(rot);

	if (t >= 1.0f) {
		isStartCameraEasing_ = false;
		player_->SetPlayerControlEnabled(true); // プレイヤー操作有効化
		// 必要ならここでカメラモードを切り替え
		cameraMode_ = CameraMode::FollowPlayer;
	}
}

void GamePlayScene::UpdatePlayerOnCurve()
{
	// カーブ座標が2点以上ある場合のみ移動
	if (curvePoints_.size() >= 2) {
		// 最後まで到達したら停止
		if (curveIndex_ >= curvePoints_.size() - 1) {
			// 最終座標で停止
			player_->SetPosition(curvePoints_.back());
			return;
		}

		// 現在の区間の始点・終点
		const Vector3& start = curvePoints_[curveIndex_];
		const Vector3& end = curvePoints_[curveIndex_ + 1];

		// 線形補間
		Vector3 lerpPos;
		lerpPos.x = std::lerp(start.x, end.x, curveProgress_);
		lerpPos.y = std::lerp(start.y, end.y, curveProgress_);
		lerpPos.z = std::lerp(start.z, end.z, curveProgress_);

		// プレイヤー座標を更新
		player_->SetPosition(lerpPos);

		// 進行度を進める
		curveProgress_ += curveSpeed_;
		if (curveProgress_ >= 1.0f) {
			curveProgress_ = 0.0f;
			curveIndex_++;
			// 次の区間がなければ停止
			if (curveIndex_ >= curvePoints_.size() - 1) {
				// 最終座標で停止
				player_->SetPosition(curvePoints_.back());
			}
		}
	}
}

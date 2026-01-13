#include "GamePlayScene.h"
#include "SRFramework.h"
#include "Object3dCommon.h"
#include "PlayerChargeBullet.h"
#include "Vector3.h"
#include "Inverse.h"
#include <iostream>
#include <Multiply.h>
#include <cmath>
#include <CurveLibrary.h>
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
	
	// スタート演出用カメラ初期化
	isStartCameraEasing_ = true;
	startCameraTimer_ = 0.0f;
	cameraManager_->SetCameraPosition(startCameraPos_);
	cameraManager_->SetCameraRotation(startCameraRot_);

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
	// ゲームクリア時の更新
	UpdateGameClear();
	// ゲームオーバーじゃないとき
	if (!isGameOver_) {
		// プレイヤーの更新
		player_->Update();

		// カメラ追従更新
		// プレイヤー操作が有効なときのみカメラ追従を更新
		if (!isStartCameraEasing_) {
			UpdatePlayerFollowCamera();
			// プレイヤーの移動制限
			RestrictPlayerInsideCameraView();
		}

		// プレイヤーの弾の奥行き調整
		for (auto& bullet : player_->GetBullets()) {
			if (bullet && bullet->IsAlive()) {
				Camera* cam = cameraManager_->GetMainCamera();
				Vector3 bulletPos = bullet->GetTranslate();
				// 画面外の場合は消す
				if (!IsInCameraView(bulletPos)) {
					bullet->SetIsAlive(false);
				}
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
		// スタート演出中でなければ敵の奥行き調整を行う
		if (!isStartCameraEasing_) {
			Camera* cam = cameraManager_->GetMainCamera();

			// 敵の奥行き調整
			Vector3 enemyPos = enemy->GetPosition();
			bool inView = IsInCameraView(enemyPos);
			enemy->SetControlEnabled(inView);
			enemy->SetZ(cam->GetTranslate().z + 10.0f);

			// 敵の弾の奥行き調整
			for (auto& bullet : enemy->GetBullets()) {
				if (bullet && bullet->IsAlive()) {
					Vector3 bulletPos = bullet->GetPosition();
					bulletPos.z = cam->GetTranslate().z + 10.0f; // プレイヤーと同じ奥行
					bullet->SetPosition(bulletPos);
				}
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

	// スタート演出カメラ初期化
	if (isStartCameraEasing_) {
		player_->SetPlayerControlEnabled(false); // プレイヤー操作無効化
		// ルートカメラの更新開始
		UpdateStartCameraEasing();
		cameraManager_->Update();
		Object3dCommon::GetInstance()->SetDefaultCamera(cameraManager_->GetMainCamera());
		return; // 他の更新をスキップ（必要に応じて調整）
	}

	// 通常のカメラ更新（ゲームクリア時にカメライージング中なら上で更新済み）
	if (!gameClearCameraMoving_) {
		cameraManager_->Update();
	}
	Object3dCommon::GetInstance()->SetDefaultCamera(cameraManager_->GetMainCamera());

	// スプライトの更新
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
		// 最終到達点（EnemySpawn）
		Vector3 anchor = enemyData.translation;

		// カーブ取得（ローカル）
		CurveData curve = CurveLibrary::Get(enemyData.move);

		// カーブをワールド座標へ変換
		for (auto& p : curve.points) {
			p += anchor;
		}

		// 初期位置はカーブの先頭
		newEnemy->SetPosition(curve.points.front());

		newEnemy->StartCurveMove(curve);
		//newEnemy->SetMoveType(enemyData.move);
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
	startCameraTimer_ += GamePlayDefaults::kDeltaTime60Hz; // フレームレート固定なら
	float t = std::clamp(startCameraTimer_ / kStartCameraDuration_, 0.0f, 1.0f);

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

	if (t >= 1.0f) {
		isStartCameraEasing_ = false;
		player_->SetPlayerControlEnabled(true); // プレイヤー操作有効化
		for (auto& enemy : enemies_) {
			enemy->SetControlEnabled(true);
		}
		// 必要ならここでカメラモードを切り替え
		cameraMode_ = CameraMode::Free;
	}
}

void GamePlayScene::UpdateCameraOnCurve()
{
	// 点が2つ以下なら動かない
	if (curvePoints_.size() < 2) return;

	// 最後の制御点到達 → ゲームクリア
	if (curveIndex_ >= curvePoints_.size() - 1) {
		cameraManager_->SetCameraPosition(curvePoints_.back());
		isGameClear_ = true;
		return;
	}

	// ------- 時間制御 ----------
	// 次の制御点までの時間(JSON time)
	float duration = levelData_->curves[0].times[curveIndex_ + 1];
	if (curveIndex_ == 0) duration = levelData_->curves[0].times[1]; // 最初はスキップ

	segmentTimer_ += kDeltaTime_;
	float t = std::clamp(segmentTimer_ / duration, 0.0f, 1.0f);

	// ------- Catmull-Rom 用 index ----------
	size_t p0 = std::max(static_cast<int32_t>(curveIndex_) - 1, 0);
	size_t p1 = curveIndex_;
	size_t p2 = curveIndex_ + 1;
	size_t p3 = std::min(static_cast<int32_t>(curvePoints_.size()) - 1, static_cast<int32_t>(curveIndex_) + 2);

	// ------- Catmull-Rom 補間 ----------
	auto catmullRom = [&](const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d, float t) {
		float t2 = t * t;
		float t3 = t2 * t;

		return Vector3(
			0.5f * (2.0f * b.x + (-a.x + c.x) * t + (2.0f * a.x - 5.0f * b.x + 4.0f * c.x - d.x) * t2 + (-a.x + 3.0f * b.x - 3.0f * c.x + d.x) * t3),
			0.5f * (2.0f * b.y + (-a.y + c.y) * t + (2.0f * a.y - 5.0f * b.y + 4.0f * c.y - d.y) * t2 + (-a.y + 3.0f * b.y - 3.0f * c.y + d.y) * t3),
			0.5f * (2.0f * b.z + (-a.z + c.z) * t + (2.0f * a.z - 5.0f * b.z + 4.0f * c.z - d.z) * t2 + (-a.z + 3.0f * b.z - 3.0f * c.z + d.z) * t3)
		);
		};

	Vector3 newPos = catmullRom(
		curvePoints_[p0],
		curvePoints_[p1],
		curvePoints_[p2],
		curvePoints_[p3],
		t
	);

	cameraManager_->SetCameraPosition(newPos);

	// 次区間へ
	if (t >= 1.0f) {
		curveIndex_++;
		segmentTimer_ = 0.0f;
	}
}

void GamePlayScene::RestrictPlayerInsideCameraView() {
	Camera* cam = cameraManager_->GetMainCamera();
	Matrix4x4 viewProj = cam->GetViewProjectionMatrix();

	Vector3 playerPos = player_->GetPosition();

	// ワールド座標 → クリップ座標
	Vector4 clipPos = Multiply::Multiply(viewProj, Vector4{ playerPos.x, playerPos.y, playerPos.z, 1.0f });

	// NDC座標へ変換
	Vector3 ndcPos = {
		clipPos.x / clipPos.w,
		clipPos.y / clipPos.w,
		clipPos.z / clipPos.w
	};

	// NDC空間でプレイヤーが -1～1 に収まるように制限
	ndcPos.x = std::clamp(ndcPos.x, -0.9f, 0.9f);  // 少し余裕を持たせる
	ndcPos.y = std::clamp(ndcPos.y, -0.9f, 0.9f);

	// NDC → クリップ座標へ戻す
	Vector4 newClipPos = { ndcPos.x * clipPos.w, ndcPos.y * clipPos.w, ndcPos.z * clipPos.w, clipPos.w };

	// ワールド座標に戻す（逆行列で）
	Matrix4x4 invViewProj = Inverse::Inverse(viewProj);
	Vector4 newWorldPos = Multiply::Multiply(invViewProj, newClipPos);

	// プレイヤー位置を更新
	Vector3 clampedWorldPos = {
		newWorldPos.x / newWorldPos.w,
		newWorldPos.y / newWorldPos.w,
		newWorldPos.z / newWorldPos.w
	};  
	player_->SetPosition(clampedWorldPos);
}

void GamePlayScene::UpdatePlayerFollowCamera()
{
	Vector3 prevCamPos = cameraManager_->GetMainCamera()->GetTranslate();

	if (!isStartCameraEasing_) {
		if (isGameClear_) return;
		// カメラをカーブに沿って移動
		UpdateCameraOnCurve();
	}
	Camera* cam = cameraManager_->GetMainCamera();
	Vector3 camPos = cam->GetTranslate();

	// カメラの移動量（前フレームとの差）
	Vector3 camMove = {
		camPos.x - prevCamPos.x,
		camPos.y - prevCamPos.y,
		camPos.z - prevCamPos.z
	};

	// プレイヤーの位置を更新（カメラ移動分を打ち消す）
	Vector3 playerPos = player_->GetPosition();
	playerPos.x += camMove.x;   // ← X/Y軸で追従
	playerPos.y += camMove.y;

	// カメラから一定距離に固定（例: +10）
	playerPos.z = camPos.z + 10.0f;

	player_->SetPosition(playerPos);
}

bool GamePlayScene::IsInCameraView(const Vector3& worldPos)
{
	Camera* cam = cameraManager_->GetMainCamera();
	Matrix4x4 viewProj = cam->GetViewProjectionMatrix();

	// ワールド → クリップ
	Vector4 clip = Multiply::Multiply(viewProj, Vector4{ worldPos.x, worldPos.y, worldPos.z, 1.0f });
	if (clip.w == 0.0f) return false;

	// NDCへ変換
	Vector3 ndc = {
		clip.x / clip.w,
		clip.y / clip.w,
		clip.z / clip.w
	};

	// NDCが -1～1 の範囲なら画面内
	return (ndc.x >= -1.0f && ndc.x <= 1.0f &&
		ndc.y >= -1.0f && ndc.y <= 1.0f &&
		ndc.z >= 0.0f && ndc.z <= 1.0f);
}

void GamePlayScene::UpdateGameClear()
{
	// ゲームクリア時の処理
	if (isGameClear_) {
		// プレイヤー操作を無効化
		player_->SetPlayerControlEnabled(false);
		// 初回に待機を開始
		if (!gameClearCameraWaiting_ && !gameClearCameraMoving_ && !gameClearPlayerLaunched_ && !gameClearFadeStarted_) {
			gameClearCameraWaiting_ = true;
			gameClearWaitTimer_ = GamePlayDefaults::kDeltaTime60Hz * 120.0f; // 2秒待つ
		}

		// 待機タイマー処理
		if (gameClearCameraWaiting_) {
			gameClearWaitTimer_ -= GamePlayDefaults::kDeltaTime60Hz;
			if (gameClearWaitTimer_ <= 0.0f) {
				gameClearCameraWaiting_ = false;
				gameClearCameraMoving_ = true;
				gameClearMoveTimer_ = 0.0f;
				// カメラの開始位置と目標位置を設定
				gameClearCamStartPos_ = cameraManager_->GetMainCamera()->GetTranslate();
				gameClearCamTargetPos_ = player_->GetPosition() + Vector3{ 0.0f, 0.0f, -5.0f };
			}
		}
		// カメラ移動（イージング）
		else if (gameClearCameraMoving_) {
			gameClearMoveTimer_ += GamePlayDefaults::kDeltaTime60Hz;
			float t = std::clamp(gameClearMoveTimer_ / kGameClearMoveDuration_, 0.0f, 1.0f);
			// easeInOutCubic
			float easeT;
			if (t < 0.5f) {
				easeT = 4.0f * t * t * t;
			}
			else {
				float p = (t - 1.0f);
				easeT = 1.0f + 4.0f * p * p * p;
			}
			Vector3 pos = {
				std::lerp(gameClearCamStartPos_.x, gameClearCamTargetPos_.x, easeT),
				std::lerp(gameClearCamStartPos_.y, gameClearCamTargetPos_.y, easeT),
				std::lerp(gameClearCamStartPos_.z, gameClearCamTargetPos_.z, easeT)
			};
			// カメラはクリア演出用に制御する
			cameraMode_ = CameraMode::CenterPlayer;

			cameraManager_->SetCameraPosition(pos);
			// カメラ更新とデフォルトカメラ書き込み（見た目に即座に反映）
			cameraManager_->Update();
			Object3dCommon::GetInstance()->SetDefaultCamera(cameraManager_->GetMainCamera());

			// スペースキーでプレイヤー発射開始（テキストはスペース押下で消す）
			if (input_->TriggerKey(DIK_SPACE) && !gameClearPlayerLaunched_) {
				gameClearPlayerLaunched_ = true;
				// 表示中ならテキストを消す（押したタイミングで即時非表示）
				gameClearTextVisible_ = false;
			}

			// イージング完了時の処理
			if (t >= 1.0f) {
				gameClearCameraMoving_ = false;
				// 最終位置を確定
				cameraManager_->SetCameraPosition(gameClearCamTargetPos_);

				// イージング終了直後にテキストを表示する
				// カメラ位置から Y +1.2 の位置に表示
				gameClearTextVisible_ = true;

				// 即時に位置を設定しておく（Draw 側でも毎フレーム更新します）
				if (gameClearText_) {
					Camera* cam = cameraManager_->GetMainCamera();
					if (cam) {
						Vector3 camPos = cam->GetTranslate();
						gameClearText_->SetTranslate(camPos + Vector3{ 0.0f, 1.2f, 0.0f });
						gameClearText_->Update();
					}
				}
			}
		}
	}

	// プレイヤー発射（ゲームクリア中にSPACEで発射した場合の挙動）
	if (gameClearPlayerLaunched_) {
		// カメラはクリア演出用に制御する
		cameraMode_ = CameraMode::DynamicFollow;
		// プレイヤーを +X 方向に飛ばす（簡易的に毎フレーム位置更新）
		Vector3 pos = player_->GetPosition();
		const float dt = GamePlayDefaults::kDeltaTime60Hz;
		pos.x += kGameClearPlayerLaunchSpeed_ * dt;
		player_->SetPosition(pos);

		// フェード開始を1秒遅延させる
		static float sFadeDelayTimer_ = 0.0f;
		static bool sFadeTimerInitialized_ = false;

		// 初期化フラグが無ければ初期化
		if (!sFadeTimerInitialized_) {
			sFadeDelayTimer_ = 1.0f;
			sFadeTimerInitialized_ = true;
		}

		// タイマーを進め、0以下になったらフェード開始
		if (!gameClearFadeStarted_) {
			sFadeDelayTimer_ -= dt;
			if (sFadeDelayTimer_ <= 0.0f) {
				gameClearFadeStarted_ = true;
				// フェードを開始してタイトルへ戻るコールバック（重複防止済み）
				fadeManager_->FadeInStart(GamePlayDefaults::kDeltaTime60Hz * 1.2f, [this]() { // 0.02f相当
					SetSceneNo(TITLE);
					});
			}
		}
	}
}
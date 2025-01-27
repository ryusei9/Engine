#include "GameScene.h"
#include <Length.h>
#include <Subtract.h>
#include <Add.h>
#include <Lerp.h>
#include <imgui.h>

GameScene::~GameScene()
{
	delete object3dCommon_;
	delete spriteCommon_;
	delete camera_;
	for (auto& object3d : object3ds) {
		delete object3d;
	}
	for (auto& sprite : sprites) {
		delete sprite;
	}
	/*delete playerModel;
	delete bulletModel;
	delete player_;
	delete enemyModel;*/
	for (auto& enemy : enemies_) {
		delete enemy;
	}
	delete skySphere_;
	/*for (auto& bullet : enemyBullets_) {
		delete bullet;
	}*/
}

void GameScene::Initialize()
{
	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();

	object3dCommon_ = new Object3dCommon;
	object3dCommon_->Initialize(dxCommon_);

	spriteCommon_ = new SpriteCommon;
	spriteCommon_->Initialize(dxCommon_);

	// SRVマネージャの初期化
	/*srvManager_ = SrvManager::GetInstance();
	srvManager_->Initialize(dxCommon_);*/

	// テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(dxCommon_, srvManager_->GetInstance());

	// 3Dモデルマネージャの初期化
	ModelManager::GetInstance()->Initialize(dxCommon_);

	ModelManager::GetInstance()->LoadModel("title.obj");

	camera_ = new Camera;
	camera_->SetRotate({ 0.0f,0.0f,0.0f });
	camera_->SetTranslate({ 0.0f,0.0f,-10.0f });
	object3dCommon_->SetDefaultCamera(camera_);

	for (uint32_t i = 0; i < 5; ++i) {
		if (i < 4) {
			Sprite* sprite = new Sprite();
			sprite->Initialize(spriteCommon_, dxCommon_, "resources/uvChecker.png");
			sprites.push_back(sprite);
		}
		else {
			Sprite* sprite = new Sprite();
			sprite->Initialize(spriteCommon_, dxCommon_, "resources/monsterBall.png");
			sprites.push_back(sprite);
		}
	}

	//std::vector<Model*> models;
	for (uint32_t i = 0; i < 2; ++i) {
		Object3d* object3d = new Object3d();
		object3d->Initialize(object3dCommon_);
		object3ds.push_back(object3d);
	}
	// 初期化済みの3Dオブジェクトにモデルを紐づける
	object3ds[0]->SetModel("plane.obj");
	object3ds[1]->SetModel("axis.obj");

	// プレイヤーの初期化
	playerModel = new Object3d();
	playerModel->Initialize(object3dCommon_);
	playerModel->SetModel("player.obj");

	bulletModel = std::make_unique<Object3d>();
	bulletModel->Initialize(object3dCommon_);
	bulletModel->SetModel("player_bullet.obj");

	player_ = new Player();
	player_->Initialize(playerModel, bulletModel.get());

	enemyModel = new Object3d();
	enemyModel->Initialize(object3dCommon_);
	enemyModel->SetModel("mori.obj");

	enemyBulletModel = new Object3d();
	enemyBulletModel->Initialize(object3dCommon_);
	enemyBulletModel->SetModel("enemy_bullet.obj");

	skySphereModel = new Object3d();
	skySphereModel->Initialize(object3dCommon_);
	skySphereModel->SetModel("sky_sphere.obj");

	titleModel = new Object3d();
	titleModel->Initialize(object3dCommon_);
	titleModel->SetModel("title.obj");
	titleModel->SetTranslate({ 0.0f,1.0f,-3.0f });
	titleModel->SetScale({ 0.5f,0.5f,0.5f });
	titleModel->SetRotate({ -0.2f,0.0f,0.0f });

	titleGuideModel = new Object3d();
	titleGuideModel->Initialize(object3dCommon_);
	titleGuideModel->SetModel("titleGuide.obj");
	titleGuideModel->SetTranslate({ 0.0f,-1.0f,-0.0f });
	titleGuideModel->SetScale({ 0.5f,0.5f,0.5f });
	titleGuideModel->SetRotate({ 0.1f,0.0f,0.0f });

	gameOverModel = new Object3d();
	gameOverModel->Initialize(object3dCommon_);
	gameOverModel->SetModel("GAMEOVER.obj");
	gameOverModel->SetTranslate({ 0.0f,1.0f,-3.0f });
	gameOverModel->SetScale({ 0.5f,0.5f,0.5f });
	gameOverModel->SetRotate({ -0.2f,0.0f,0.0f });

	clearModel = new Object3d();
	clearModel->Initialize(object3dCommon_);
	clearModel->SetModel("GAMECLEAR.obj");
	clearModel->SetTranslate({ 0.0f,1.0f,-3.0f });
	clearModel->SetScale({ 0.5f,0.5f,0.5f });
	clearModel->SetRotate({ -0.2f,0.0f,0.0f });

	/*tutorialModel = new Object3d();
	tutorialModel->Initialize(object3dCommon_);
	tutorialModel->SetModel("tutorial.obj");
	tutorialModel->SetTranslate({ 2.0f,-2.0f,1.0f });
	tutorialModel->SetScale({ 0.5f,0.5f,0.5f });
	tutorialModel->SetRotate({ 0.2f,0.0f,0.0f });*/
	tutorialSprite = new Sprite();
	tutorialSprite->Initialize(spriteCommon_, dxCommon_, "resources/tutorial.png");
	tutorialSprite->SetPosition({ 760.0f,600.0f });

	enemy_ = new Enemy();
	enemy_->Initialize(enemyModel, enemyBulletModel, enemyPosition_);
	enemy_->SetGameScene(this);
	enemy_->SetPlayer(player_);

	skySphere_ = new SkySphere();
	skySphere_->Initialize(skySphereModel);



	//enemies_.push_back(enemy);
	//moveCameraTransform.translate = enemy_->GetWorldPosition();
	moveCameraTransform.translate.y = -10.0f;
	moveCameraTransform.rotate.y = 1.5f;
	moveCameraTransform.rotate.x = -1.5f;
	// 敵発生データの読み込み
	//LoadEnemyPopData();
	timer_ = 0.0f;
}

void GameScene::Update()
{

	// デスフラグの立った敵を削除
	enemies_.remove_if([](Enemy* enemy) {
		if (enemy->IsDead()) {
			delete enemy;
			return true;
		}
		return false;
		});

	// 入力の更新
	input_->Update();

	skySphere_->Update();
	camera_->Update();
	switch (scene) {
	case TITLE:
		titleModel->Update();
		titleGuideModel->Update();
		moveCameraTransform = {
		{1.0f,1.0f,1.0f},
		{-1.5f,1.5f,0.0f},
		{2.0f,-10.0f,0.0f}
		};
		player_->SetInitialize();
		enemy_->SetInitialize(enemyPosition_);
		timer_ = 0.0f;
		camera_->SetTranslate(cameraTransform.translate);
		camera_->SetRotate(cameraTransform.rotate);
		//camera_->Update();
		if (input_->TriggerKey(DIK_SPACE)) {
			scene = GAME;
		}
		break;
	case GAME:
		// タイマーを更新
		timer_ += 1.0f / 60.0f; // フレームレートが60FPSの場合

		// 3秒待ってから線形補間を実行
		if (timer_ >= waitTime_) {
			moveCameraTransform.translate = Lerp(moveCameraTransform.translate, cameraTransform.translate, 0.01f);
			moveCameraTransform.rotate = Lerp(moveCameraTransform.rotate, cameraTransform.rotate, 0.01f);
		}
		camera_->SetTranslate(moveCameraTransform.translate);
		camera_->SetRotate(moveCameraTransform.rotate);
		//camera_->Update();
		
		if (enemy_->IsDead()) {
			scene = CLEAR;
		}

		player_->Update();

		if (player_->IsDead()) {
			scene = GAMEOVER;
		}
		
		enemy_->Update();
		
		tutorialSprite->Update();
		///tutorialModel->Update();
		// 衝突判定
		CheckAllCollisions();
		break;
	case GAMEOVER:
		gameOverModel->Update();
		titleGuideModel->Update();
		camera_->SetTranslate(cameraTransform.translate);
		camera_->SetRotate(cameraTransform.rotate);
		if (input_->TriggerKey(DIK_SPACE)) {
			scene = TITLE;
		}
		break;
	case CLEAR:
		clearModel->Update();
		titleGuideModel->Update();
		camera_->SetTranslate(cameraTransform.translate);
		camera_->SetRotate(cameraTransform.rotate);
		//camera_->Update();
		if (input_->TriggerKey(DIK_SPACE)) {
			scene = TITLE;
		}
		break;
	}
}

void GameScene::Draw()
{
	//srvManager_->PreDraw();
	// 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックコマンドを積む
	object3dCommon_->DrawSettings();
	// 全てのobject3d個々の描画
	skySphere_->Draw();
	switch (scene) {
	case TITLE:
		titleModel->Draw();
		titleGuideModel->Draw();
		break;
	case GAME:
		
		
		player_->Draw();
		// 敵キャラの描画

		enemy_->Draw();
		
		//tutorialModel->Draw();
		break;
	case GAMEOVER:
		gameOverModel->Draw();
		titleGuideModel->Draw();
		break;
	case CLEAR:
		clearModel->Draw();
		titleGuideModel->Draw();
		break;
	}
	// Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
	spriteCommon_->DrawSettings();
	switch (scene) {
	case TITLE:
		break;
	case GAME:
		tutorialSprite->Draw();
		break;
	case GAMEOVER:
		break;
	case CLEAR:
		break;
	}
	

}

void GameScene::CheckAllCollisions()
{
	// 判定対象AとBの座標
	Vector3 posA{}, posB{};

	// 自弾リストの取得
	const std::list<PlayerBullet*>& playerBullets = player_->GetBullets();
	// 敵弾リストの取得
	const std::list<EnemyBullet*>& enemyBullets = GetEnemyBullets();

#pragma region 自キャラと敵弾の当たり判定
	// 自キャラの座標
	posA = player_->GetWorldPosition();

	// 自キャラと敵弾全ての当たり判定
	for (EnemyBullet* bullet : enemyBullets) {
		// 敵弾の座標
		posB = bullet->GetWorldPosition();
		// 距離
		float length = Length(Subtract(posB, posA));
		if (length <= (playerRadius_ + enemyBulletRadius_)) {
			// 自キャラの衝突時コールバックを呼び出す
			player_->OnCollision();
			// 敵弾の衝突時コールバックを呼び出す
			bullet->OnCollision();
		}
	}
#pragma endregion

#pragma region 自弾と敵キャラの当たり判定

	// 敵キャラと自弾全ての当たり判定

	for (PlayerBullet* bullet : playerBullets) {
		// 敵キャラの座標
		posA = enemy_->GetWorldPosition();
		// 自弾の座標
		posB = bullet->GetWorldPosition();
		// 距離
		float length = Length(Subtract(posB, posA));
		if (length <= (playerBulletRadius_ + enemyRadius_)) {
			// 敵キャラの衝突時コールバックを呼び出す
			enemy_->OnCollision();
			// 自弾の衝突時コールバックを呼び出す
			bullet->OnCollision();
		}
	}

#pragma endregion

#pragma region 自弾と敵弾の当たり判定
	// 自弾と敵弾全ての当たり判定
	for (PlayerBullet* bulletA : playerBullets) {
		for (EnemyBullet* bulletB : enemyBullets) {
			// 自弾の座標
			posA = bulletA->GetWorldPosition();
			// 敵弾の座標
			posB = bulletB->GetWorldPosition();
			// 距離
			float length = Length(Subtract(posB, posA));
			if (length <= (playerBulletRadius_ + enemyBulletRadius_)) {
				// 敵弾の衝突時コールバックを呼び出す
				bulletA->OnCollision();
				// 自弾の衝突時コールバックを呼び出す
				bulletB->OnCollision();
			}
		}
	}
#pragma endregion
}

void GameScene::AddEnemyBullet(EnemyBullet* enemyBullet)
{
	// リストに登録する
	enemyBullets_.push_back(enemyBullet);
}

void GameScene::LoadEnemyPopData()
{
	// ファイルを開く
	std::ifstream file;
	file.open("resources/enemyPop.csv");
	assert(file.is_open());

	// ファイルの内容を文字列ストリームにコピー
	enemyPopCommands << file.rdbuf();

	// ファイルを閉じる
	file.close();
}

void GameScene::UpdateEnemyPopCommands()
{
	// 待機処理
	if (isWait) {
		kWaitTimer_--;
		if (kWaitTimer_ <= 0) {
			// 待機完了
			isWait = false;
		}
		return;
	}
	// 1行分の文字列を入れる変数
	std::string line;

	// コマンド実行ループ
	while (getline(enemyPopCommands, line)) {
		// 1行分の文字列をストリームに変換して解析しやすくなる
		std::istringstream line_stream(line);

		std::string word;
		// ,区切りで行の先頭文字列を取得
		getline(line_stream, word, ',');

		// "//"から始まる行はコメント
		if (word.find("//") == 0) {
			// コメント行を飛ばす
			continue;
		}
		// POPコマンド
		if (word.find("POP") == 0) {
			// x座標
			getline(line_stream, word, ',');
			float x = (float)std::atof(word.c_str());

			// y座標
			getline(line_stream, word, ',');
			float y = (float)std::atof(word.c_str());

			// z座標
			getline(line_stream, word, ',');
			float z = (float)std::atof(word.c_str());

			// 敵を発生させる
			enemyPop(Vector3(x, y, z));
			// WAITコマンド
		}
		else if (word.find("WAIT") == 0) {
			getline(line_stream, word, ',');

			// 待ち時間
			int32_t waitTime = atoi(word.c_str());

			// 待機開始
			isWait = true;
			kWaitTimer_ = waitTime;

			// コマンドループを抜ける
			break;
		}
	}
}

void GameScene::enemyPop(Vector3 translation)
{
	// 敵キャラの生成
	enemy_ = new Enemy();
	// 敵キャラの初期化
	enemy_->Initialize(enemyModel, enemyBulletModel, translation);
	// 敵キャラにゲームシーンを渡す
	enemy_->SetGameScene(this);

	// 敵キャラに自キャラのアドレスを渡す
	enemy_->SetPlayer(player_);

	enemies_.push_back(enemy_);

}

void GameScene::DrawImGui()
{
	ImGui::Begin("GameScene");

	ImGui::Text("GameScene");

	player_->ImGuiDraw();

	enemy_->ImGuiDraw();

	ImGui::End();
}

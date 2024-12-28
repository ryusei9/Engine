#include "GameScene.h"
#include <Length.h>
#include <Subtract.h>
#include <Add.h>

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
}

void GameScene::Initialize()
{
	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();

	object3dCommon_ = new Object3dCommon;
	object3dCommon_->Initialize(dxCommon_);

	spriteCommon_ = new SpriteCommon;
	spriteCommon_->Initialize(dxCommon_);

	// テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(dxCommon_);

	// 3Dモデルマネージャの初期化
	ModelManager::GetInstance()->Initialize(dxCommon_);

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
	playerModel->SetModel("plane.obj");

	bulletModel = new Object3d();
	bulletModel->Initialize(object3dCommon_);
	bulletModel->SetModel("axis.obj");

	player_ = new Player();
	player_->Initialize(playerModel,bulletModel);

	// 敵発生データの読み込み
	LoadEnemyPopData();
}

void GameScene::Update()
{
	// 入力の更新
	input_->Update();
	// 操作
	/*if (input_->PushKey(DIK_RIGHTARROW)) {
		cameraTransform.translate.x -= 0.1f;

	}
	if (input_->PushKey(DIK_LEFTARROW)) {
		cameraTransform.translate.x += 0.1f;
	}*/
	camera_->SetTranslate(cameraTransform.translate);
	camera_->Update();

	for (uint32_t i = 0; i < sprites.size();++i) {

		sprites[i]->Update();

		// 現在の座標を変数で受ける
		Vector2 position = sprites[i]->GetPosition();

		// 座標を変更する
		
		// 変更を反映する
		sprites[i]->SetPosition({ 200.0f * i });

		// 角度を変更させるテスト
		float rotation = sprites[i]->GetRotation();

		

		sprites[i]->SetRotation(rotation);

		// 色を変化させるテスト
		Vector4 color = sprites[i]->GetColor();

		

		sprites[i]->SetColor(color);

		// サイズを変化させるテスト
		Vector2 size = sprites[i]->GetSize();
		

		sprites[i]->SetSize({ 100.0f,100.0f });

		// 反転X
		bool isFlipX = sprites[i]->GetIsFlipX();
		

		sprites[i]->SetIsFlipX(isFlipX);

		bool isFlipY = sprites[i]->GetIsFlipY();
		

		sprites[i]->SetIsFlipY(isFlipY);
	}

	for (uint32_t i = 0; i < object3ds.size();++i) {
		object3ds[i]->Update();
		// 現在の座標を変数で受ける
		Vector3 position[2];
		position[i] = object3ds[i]->GetTranslate();
		// 座標を変更する
		
		position[0].x = -2.0f;
		position[1].x = 2.0f;
		object3ds[i]->SetTranslate(position[i]);

		// 角度を変更させるテスト
		Vector3 rotation[2];
		rotation[i] = object3ds[i]->GetRotate();

		
		rotation[0].y += 0.01f;
		rotation[1].z += 0.01f;
		object3ds[i]->SetRotate(rotation[i]);

		// 拡縮を変更するテスト
		Vector3 scale[2];
		scale[i] = object3ds[i]->GetScale();
		
		object3ds[i]->SetScale(scale[i]);
	}
	player_->Update();
}

void GameScene::Draw()
{
	// 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックコマンドを積む
	object3dCommon_->DrawSettings();
	// 全てのobject3d個々の描画
	/*for (auto& object3d : object3ds) {
		object3d->Draw();
	}*/
	player_->Draw();
	// Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
	spriteCommon_->DrawSettings();

	/*for (auto& sprite : sprites) {
		sprite->Draw();
	}*/
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
	for (Enemy* enemy : enemies_) {
		for (PlayerBullet* bullet : playerBullets) {
			// 敵キャラの座標
			posA = enemy->GetWorldPosition();
			// 自弾の座標
			posB = bullet->GetWorldPosition();
			// 距離
			float length = Length(Subtract(posB, posA));
			if (length <= (playerBulletRadius_ + enemyRadius_)) {
				// 敵キャラの衝突時コールバックを呼び出す
				enemy->OnCollision();
				// 自弾の衝突時コールバックを呼び出す
				bullet->OnCollision();
			}
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
	file.open("Resources/enemyPop.csv");
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
	enemy_->Initialize(enemyModel, translation);
	// 敵キャラにゲームシーンを渡す
	enemy_->SetGameScene(this);

	// 敵キャラに自キャラのアドレスを渡す
	enemy_->SetPlayer(player_);

	enemies_.push_back(enemy_);

}

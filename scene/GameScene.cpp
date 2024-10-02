#include "GameScene.h"
#include "AxisIndicator.h"
#include "ImGuiManager.h"
#include "TextureManager.h"
#include "WorldTransform.h"
#include <cassert>
#include <fstream>

GameScene::GameScene() {}

GameScene::~GameScene() {
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	for (EnemyBullet* bullet : enemyBullets_) {
		delete bullet;
	}
	delete railCamera_;
	delete modelSkydome_;
	delete skydome_;
	delete mathMatrix_;
	delete debugCamera_;
	delete player_;
	delete model_;
}

void GameScene::Initialize() {

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();

	///======================
	/// 3Dモデルの初期化
	///======================

	// テクスチャ読み込み
	textureHandle_ = TextureManager::Load("zako.png");

	// モデルデータの生成
	model_ = Model::Create();

	// ビュープロジェクションの初期化
	viewProjection_.farZ = 1000.0f;
	viewProjection_.Initialize();

	// レティクルのテクスチャ
	TextureManager::Load("reticle.png");

	// 自キャラの生成
	player_ = new Player();

	// 自キャラの初期化
	Vector3 playerPosition(0, 0, 15.0f);
	player_->Initialize(model_, textureHandle_, playerPosition, &viewProjection_);

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	// 軸方向表示を有効にする
	AxisIndicator::GetInstance()->SetVisible(true);
	// 軸方向表示が参照するビュープロジェクションを指定する(アドレス渡し)
	AxisIndicator::GetInstance()->SetTargetViewProjection(&viewProjection_);

	mathMatrix_ = new MathMatrix;

	// 天球の生成
	skydome_ = new Skydome;

	// 3Dモデルの生成
	modelSkydome_ = Model::CreateFromOBJ("celestial_sphere", true);

	// 天球の初期化
	skydome_->Initialize(modelSkydome_, &viewProjection_);

	// レールカメラの生成
	railCamera_ = new RailCamera;

	// レールカメラの初期化
	railCamera_->Initialize(cameraTranslation, cameraRotate);

	// 自キャラとレールカメラの親子関係を結ぶ
	player_->SetParent(&railCamera_->GetWorldTransform());

	LoadEnemyPopData();

	
}

///======================
/// 更新
///======================
void GameScene::Update() {
	// デスフラグの立った弾を削除
	enemyBullets_.remove_if([](EnemyBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});
	// デスフラグの立った敵を削除
	enemies_.remove_if([](Enemy* enemy) {
		if (enemy->IsDead()) {
			delete enemy;
			return true;
		}
		return false;
	});
	// 自キャラの更新
	player_->Update();
	// デバッグカメラの更新
	debugCamera_->Update();
	
	UpdateEnemyPopCommands();

	for (Enemy* enemy : enemies_) {
		enemy->Update();
	}
#ifdef _DEBUG
	if (input_->TriggerKey(DIK_C)) {
		isDebugCameraActive_ = true;
	}
#endif
	// カメラの処理
	if (isDebugCameraActive_) {
		// デバッグカメラの更新
		debugCamera_->Update();
		// debugCamera_->DebugCamera::SetFarZ(1000.0f);
		viewProjection_.matView = debugCamera_->DebugCamera::GetView();
		viewProjection_.matProjection = debugCamera_->DebugCamera::GetProjection();
		// ビュープロジェクション行列の転送
		viewProjection_.TransferMatrix();
	} else {
		// ビュープロジェクション行列の更新と転送
		viewProjection_.UpdateMatrix();
	}
	CheckAllCollisions();
	skydome_->Update();

	// レールカメラの更新
	railCamera_->Update();

	// レールカメラのビュープロジェクションをコピー
	viewProjection_.matView = railCamera_->GetView();
	viewProjection_.matProjection = railCamera_->GetProjection();

	// ビュープロジェクション行列の転送
	viewProjection_.TransferMatrix();

	// 弾更新
	for (EnemyBullet* bullet : enemyBullets_) {
		bullet->Update();
	}

	
}

///======================
/// 描画
///======================
void GameScene::Draw() {

	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに背景スプライトの描画処理を追加できる
	/// </summary>

	// スプライト描画後処理
	Sprite::PostDraw();
	// 深度バッファクリア
	dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画前処理
	Model::PreDraw(commandList);

	/// <summary>
	/// ここに3Dオブジェクトの描画処理を追加できる
	/// </summary>

	// プレイヤーの描画
	player_->Draw();
	// 敵キャラの描画
	for (Enemy* enemy : enemies_) {
		enemy->Draw(viewProjection_);
	}
	skydome_->Draw();
	// 弾描画
	for (EnemyBullet* bullet : enemyBullets_) {
		bullet->Draw(viewProjection_);
	}

	// 3Dオブジェクト描画後処理
	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>
	player_->DrawUI();

	// スプライト描画後処理
	Sprite::PostDraw();

#pragma endregion
}

void GameScene::CheckAllCollisions() {
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
		float length = mathMatrix_->MathMatrix::Length(mathMatrix_->MathMatrix::Subtract(posB, posA));
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
			float length = mathMatrix_->MathMatrix::Length(mathMatrix_->MathMatrix::Subtract(posB, posA));
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
			float length = mathMatrix_->MathMatrix::Length(mathMatrix_->MathMatrix::Subtract(posB, posA));
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

void GameScene::AddEnemyBullet(EnemyBullet* enemyBullet) {

	// リストに登録する
	enemyBullets_.push_back(enemyBullet);
}

void GameScene::LoadEnemyPopData() {
	// ファイルを開く
	std::ifstream file;
	file.open("Resources/enemyPop.csv");
	assert(file.is_open());

	// ファイルの内容を文字列ストリームにコピー
	enemyPopCommands << file.rdbuf();

	// ファイルを閉じる
	file.close();
}

void GameScene::UpdateEnemyPopCommands() {
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
		} else if (word.find("WAIT") == 0) {
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

void GameScene::enemyPop(Vector3 translation) {
	// 敵キャラの生成
	enemy_ = new Enemy();
	// 敵キャラの初期化
	enemy_->Initialize(model_, translation);
	// 敵キャラにゲームシーンを渡す
	enemy_->SetGameScene(this);

	// 敵キャラに自キャラのアドレスを渡す
	enemy_->SetPlayer(player_);

	enemies_.push_back(enemy_);
}

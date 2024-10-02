#pragma once

#include "Audio.h"
#include "CameraController.h"
#include "DebugCamera.h"
#include "DirectXCommon.h"
#include "Enemy.h"
#include "Input.h"
#include "MathMatrix.h"
#include "Model.h"
#include "Player.h"
#include "Sprite.h"
#include "ViewProjection.h"
#include <vector>
#include <Skydome.h>
#include <RailCamera.h>
#include <sstream>

/// <summary>
/// ゲームシーン
/// </summary>
class GameScene {

public: // メンバ関数
	/// <summary>
	/// コンストクラタ
	/// </summary>
	GameScene();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~GameScene();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 衝突判定と応答
	/// </summary>
	void CheckAllCollisions();

	/// <summary>
	/// 敵弾を追加する
	/// </summary>
	void AddEnemyBullet(EnemyBullet* enemyBullet);

	// 弾リストを取得
	const std::list<EnemyBullet*>& GetEnemyBullets() const { return enemyBullets_; }

	/// <summary>
	/// 敵発生データの読み込み
	/// </summary>
	void LoadEnemyPopData();

	/// <summary>
	/// 敵発生コマンドの更新
	/// </summary>
	void UpdateEnemyPopCommands();

	/// <summary>
	/// 敵発生
	/// </summary>
	void enemyPop(Vector3 translation);

private: // メンバ変数
	DirectXCommon* dxCommon_ = nullptr;
	Input* input_ = nullptr;
	Audio* audio_ = nullptr;

	// テクスチャ
	uint32_t textureHandle_ = 0;

	Model* model_ = nullptr;

	// ビュープロジェクション
	ViewProjection viewProjection_;

	// 自キャラ
	Player* player_ = nullptr;

	// デバッグカメラ有効
	bool isDebugCameraActive_ = false;

	// デバッグカメラ
	DebugCamera* debugCamera_ = nullptr;

	// 敵
	Enemy* enemy_ = nullptr;

	Vector3 enemyPosition_ = {5.0f, 3.0f, 50.0f};

	MathMatrix* mathMatrix_ = nullptr;

	float playerRadius_ = 1.0f;

	float playerBulletRadius_ = 1.0f;

	float enemyRadius_ = 1.0f;

	float enemyBulletRadius_ = 1.0f;

	Skydome* skydome_ = nullptr;

	// 3Dモデル
	Model* modelSkydome_ = nullptr;

	// レールカメラ
	RailCamera* railCamera_ = nullptr;

	// カメラの場所
	Vector3 cameraTranslation{0, 0, -30.0f};

	// カメラの角度
	Vector3 cameraRotate{0, 0, 0};

	// 敵弾リストの取得
	std::list<EnemyBullet*> enemyBullets_;

	// 敵リスト
	std::list<Enemy*> enemies_;

	// 敵発生
	std::stringstream enemyPopCommands;

	// 待機中フラグ
	bool isWait = false;

	// 待機タイマー
	int32_t kWaitTimer_ = 0;

	/// <summary>
	/// ゲームシーン用
	/// </summary>
};

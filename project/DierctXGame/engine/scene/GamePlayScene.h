#pragma once
#include <SpriteCommon.h>
#include <DirectXCommon.h>
#include <WinApp.h>
#include <Sprite.h>
#include <Input.h>
#include <Audio.h>
#include <Vector2.h>
#include <BaseScene.h>
#include <ParticleManager.h>
#include <ParticleEmitter.h>
#include <Object3d.h>
#include <Player.h>
#include <PlayerBullet.h>
#include <Enemy.h>
#include <CollisionManager.h>
#include <LevelData.h>
#include <JsonLoader.h>
#include <Skybox.h>
#include <FadeManager.h>
#include <CameraManager.h>
// ゲームプレイシーン
class GamePlayScene : public BaseScene

{
public:
	// 初期化
	void Initialize(DirectXCommon* directXCommon, WinApp* winApp) override;

	// 更新
	void Update() override;

	// 描画
	void Draw() override;

	// 終了
	void Finalize() override;

	// ImGui描画
	void DrawImGui() override;

	// ローダーから読み込んだレベルデータからオブジェクトを生成、配置する関数
	void CreateObjectsFromLevelData();

	void DrawImGuiImportObjectsFromJson();

	void UpdateStartCameraEasing();

public:
	// ステート
	enum class GameSceneState {
		Start,
		Play,
		GameClear,
		GameOver,
		Pause
	};


	enum class CameraMode {
		Free,
		FollowPlayer,
		DynamicFollow
	};
private:

	// 衝突判定と応答
	void CheckAllCollisions();

	// スプライトコモン
	SpriteCommon* spriteCommon = nullptr;
	// ダイレクトXコモン
	DirectXCommon* directXCommon = nullptr;
	// WinApp
	WinApp* winApp = nullptr;

	std::unique_ptr<CollisionManager> collisionManager_;



	std::unique_ptr<Sprite> sprite = nullptr;

	// 入力の初期化
	Input* input = nullptr;

	Vector2 spritePosition = { 100.0f,100.0f };

	SoundData soundData1;

	ParticleManager* particleManager = nullptr;

	std::unique_ptr<ParticleEmitter> particleEmitter1 = nullptr;

	std::unique_ptr<ParticleEmitter> particleEmitter2 = nullptr;

	Vector3 particlePosition1 = { 0,0,5 };

	Vector3 particlePosition2 = { 5,0,50 };

	// ボール
	std::unique_ptr<Object3d> ball = nullptr;
	std::unique_ptr<Object3d> ground = nullptr;
	// ボールの座標
	WorldTransform ballTransform;
	WorldTransform groundTransform;

	// プレイヤー
	std::unique_ptr<Player> player_ = nullptr;

	// プレイヤーの弾
	std::list<std::unique_ptr<PlayerBullet>>* playerBullets_;

	std::list<std::unique_ptr<PlayerChargeBullet>>* playerChargeBullets_;

	// 敵の弾
	std::list<std::unique_ptr<EnemyBullet>>* enemyBullets_;

	// 敵
	std::unique_ptr<Enemy> enemy_ = nullptr;

	LevelData* levelData_ = nullptr;

	// 複数のモデルを管理するためのコンテナ
	std::unordered_map<std::string, std::unique_ptr<Model>> models;

	// 複数のオブジェクトを管理するためのコンテナ
	std::vector<std::unique_ptr<Object3d>> objects;

	// スカイボックス
	std::unique_ptr<Skybox> skybox_ = nullptr;

	// フェード演出管理
	std::unique_ptr<FadeManager> fadeManager_;

	// 複数の敵を管理するためのコンテナ
	std::vector<std::unique_ptr<Enemy>> enemies_;

	// カメラマネージャー
	std::unique_ptr<CameraManager> cameraManager_;

	CameraMode cameraMode_ = CameraMode::DynamicFollow;

	// --- スタート演出用 ---
	bool isStartCameraEasing_ = true;
	float startCameraTimer_ = 0.0f;
	const float startCameraDuration_ = 5.0f;
	Vector3 startCameraPos_ = { 70.0f,-10.0f, -20.0f };
	Vector3 startCameraRot_ = { 0.0f, 0.0f, 0.0f };
	Vector3 endCameraPos_ = { 0.0f, 1.0f, -10.0f };
	Vector3 endCameraRot_ = { 0.1f, 0.0f, 0.0f };

	std::unique_ptr<Object3d> BackToTitle;

	WorldTransform textTitle;
};


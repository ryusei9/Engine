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

	// 敵
	std::unique_ptr<Enemy> enemy_ = nullptr;

	LevelData* levelData_ = nullptr;

	// 複数のモデルを管理するためのコンテナ
	std::unordered_map<std::string, std::unique_ptr<Model>> models;

	// 複数のオブジェクトを管理するためのコンテナ
	std::vector<std::unique_ptr<Object3d>> objects;
};


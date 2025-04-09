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

private:
	// スプライトコモン
	SpriteCommon* spriteCommon = nullptr;
	// ダイレクトXコモン
	DirectXCommon* directXCommon = nullptr;
	// WinApp
	WinApp* winApp = nullptr;


	std::unique_ptr<Sprite> sprite = nullptr;

	// 入力の初期化
	std::unique_ptr<Input> input = nullptr;

	Vector2 spritePosition = { 100.0f,100.0f };

	SoundData soundData1;

	ParticleManager* particleManager = nullptr;

	std::unique_ptr<ParticleEmitter> particleEmitter1 = nullptr;

	std::unique_ptr<ParticleEmitter> particleEmitter2 = nullptr;

	Vector3 particlePosition1 = { -5,0,50 };

	Vector3 particlePosition2 = { 5,0,50 };

	// ボール
	std::unique_ptr<Object3d> ball = nullptr;
	std::unique_ptr<Object3d> ground = nullptr;
	// ボールの座標
	Transform ballTransform;
	Transform groundTransform;
};


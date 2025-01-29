#pragma once
#include <SpriteCommon.h>
#include <DirectXCommon.h>
#include <WinApp.h>
#include <Sprite.h>
#include <Input.h>
#include <Audio.h>
#include <Vector2.h>

// ゲームプレイシーン
class GamePlayeScene
{
public:
	// 初期化
	void Initialize(SpriteCommon* spriteCommon, DirectXCommon* directXCommon, WinApp* winApp);

	// 更新
	void Update();

	// 描画
	void Draw();

	// 終了
	void Finalize();

private:
	// スプライトコモン
	SpriteCommon* spriteCommon = nullptr;
	// ダイレクトXコモン
	DirectXCommon* directXCommon = nullptr;
	// WinApp
	WinApp* winApp = nullptr;


	Sprite* sprite = new Sprite();

	// 入力の初期化
	Input* input = new Input();

	Vector2 spritePosition = { 100.0f,100.0f };

	SoundData soundData1;
};


#pragma once
#include <SpriteCommon.h>
#include <DirectXCommon.h>
#include <WinApp.h>
#include <Sprite.h>
#include <Input.h>
#include <Audio.h>
#include <Vector2.h>
#include <BaseScene.h>
// ゲームプレイシーン
class GamePlayScene : public BaseScene

{
public:
	// 初期化
	void Initialize(SpriteCommon* spriteCommon, DirectXCommon* directXCommon, WinApp* winApp) override;

	// 更新
	void Update() override;

	// 描画
	void Draw() override;

	// 終了
	void Finalize() override;

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
};


#pragma once
#include <BaseScene.h>
#include <Sprite.h>
#include <FadeManager.h>
#include <Object3d.h>
#include <CameraManager.h>
#include <Input.h>
#include <Player.h>

class GameOverScene : public BaseScene
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
	// 背景スプライト
	//std::unique_ptr<Sprite> backgroundSprite_;

	// 入力の初期化
	std::unique_ptr<Input> input = nullptr;
	
	// フェードマネージャー
	std::unique_ptr<FadeManager> fadeManager_;

	// オブジェクト3D
	std::unique_ptr<Object3d> gameOverText_;

	WorldTransform gameOverTextTransform_;

	std::unique_ptr<Object3d> gameOverGuide_;

	WorldTransform gameOverGuideTransform_;

	// カメラマネージャー
	std::unique_ptr<CameraManager> cameraManager_;

	// skydome
	std::unique_ptr<Object3d> skydome_ = nullptr;

	WorldTransform skydomeTransform_;

	bool fadeStarted_ = false;

	bool returnToTitle_ = false;

	std::unique_ptr<Object3d> player_ = nullptr;

	WorldTransform playerTransform_;

};


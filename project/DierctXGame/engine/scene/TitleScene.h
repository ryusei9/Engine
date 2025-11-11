#pragma once
#include <SpriteCommon.h>
#include <DirectXCommon.h>
#include <WinApp.h>
#include <Sprite.h>
#include <Input.h>
#include <Audio.h>
#include <Vector2.h>
#include <BaseScene.h>
#include <Player.h>
#include <FadeManager.h>
#include <CameraManager.h>
class TitleScene : public BaseScene
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

	void CameraMove();

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

	// プレイヤー
	std::unique_ptr<Player> player_ = nullptr;

	WorldTransform playerTransform_;

	// タイトルロゴ
	std::unique_ptr<Object3d> titleLogo_ = nullptr;

	WorldTransform titleLogoTransform_;

	std::unique_ptr<Camera> camera_ = std::make_unique<Camera>();

	// skydome
	std::unique_ptr<Object3d> skydome_ = nullptr;

	WorldTransform skydomeTransform_;

	// フェード演出管理
	std::unique_ptr<FadeManager> fadeManager_;

	std::unique_ptr<Object3d> titleGuide_ = nullptr;

	Vector3 titleGuidePosition = { 0.0f, 0.0f, -6.000f };
	Vector3 titleGuideRotate = { -1.387f, 0.0f, 0.0f };
	Vector3 titleGuideScale = { 0.232f, 0.232f, 0.232f };

	std::unique_ptr<CameraManager> cameraManager_ = nullptr;

	// ddsファイル用のテクスチャハンドル
	UINT textureHandle_ = 0;
};


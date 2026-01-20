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
#include <Skybox.h>

/// <summary>
/// タイトルシーン	
/// </summary>
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

	// カメラ移動
	void CameraMove();

private:
	// スプライトコモン
	SpriteCommon* spriteCommon = nullptr;
	// ダイレクトXコモン
	DirectXCommon* directXCommon = nullptr;
	// WinApp
	WinApp* winApp = nullptr;

	// 入力の初期化
	std::unique_ptr<Input> input = nullptr;

	// オーディオ
	SoundData soundData1;

	// プレイヤー
	std::unique_ptr<Player> player_ = nullptr;

	// プレイヤーのワールド変換
	WorldTransform playerTransform_;

	// タイトルロゴ
	std::unique_ptr<Object3d> titleLogo_ = nullptr;

	// タイトルロゴのワールド変換
	WorldTransform titleLogoTransform_;

	// カメラ
	std::unique_ptr<Camera> camera_ = std::make_unique<Camera>();

	// skydome
	std::unique_ptr<Object3d> skydome_ = nullptr;

	// skydomeワールド変換
	WorldTransform skydomeTransform_;

	// フェード演出管理
	std::unique_ptr<FadeManager> fadeManager_;

	// ガイドオブジェクト
	std::unique_ptr<Object3d> titleGuide_ = nullptr;

	// ガイドオブジェクトのパラメータ
	Vector3 titleGuidePosition = { 0.0f, 0.0f, -6.000f };
	Vector3 titleGuideRotate = { -1.387f, 0.0f, 0.0f };
	Vector3 titleGuideScale = { 0.232f, 0.232f, 0.232f };

	// カメラマネージャー
	std::unique_ptr<CameraManager> cameraManager_ = nullptr;

	// ゲームスタートフラグ
	bool isGameStart_ = false;

	// キューブ
	
};


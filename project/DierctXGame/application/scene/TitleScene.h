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

/// 調整用定数（マジックナンバー排除）
namespace TitleDefaults {
	// カメラ
	inline constexpr Vector3 kCamPos{ 0.0f, 1.0f, -10.0f };
	inline constexpr Vector3 kCamRot{ 0.1f, 0.0f, 0.0f };

	// プレイヤー初期位置
	inline constexpr Vector3 kPlayerInitPos{ 0.0f, -0.5f, -4.0f };

	// タイトルロゴ
	inline constexpr Vector3 kTitleLogoRot{ -1.73f, 0.0f, 0.0f };
	inline constexpr Vector3 kTitleLogoPos{ 0.0f, 1.2f, 0.0f };

	// ガイド
	inline constexpr Vector3 kGuidePos{ 0.0f, 0.0f, -6.000f };
	inline constexpr Vector3 kGuideRot{ -1.387f, 0.0f, 0.0f };
	inline constexpr Vector3 kGuideScale{ 0.232f, 0.232f, 0.232f };

	// フェード
	inline constexpr float kFadeStep = 0.02f;

	// カメラ回転
	inline constexpr float kCameraRotSpeed = 0.005f; // 回転速度（ラジアン）
	inline constexpr float kCameraRadius   = 5.0f;   // プレイヤーからの距離
	inline constexpr float kCameraHeight   = 2.0f;   // カメラの高さ
	inline constexpr float kPiOver2        = 3.14159f / 2.0f;
}

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
	SpriteCommon* spriteCommon_ = nullptr;

	// ダイレクトXコモン
	DirectXCommon* directXCommon_ = nullptr;

	// WinApp
	WinApp* winApp_ = nullptr;

	// 入力の初期化
	std::unique_ptr<Input> input_ = nullptr;

	// オーディオ
	SoundData soundData1_;

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
	Vector3 titleGuidePosition_ = TitleDefaults::kGuidePos;
	Vector3 titleGuideRotate_   = TitleDefaults::kGuideRot;
	Vector3 titleGuideScale_    = TitleDefaults::kGuideScale;

	// カメラマネージャー
	std::unique_ptr<CameraManager> cameraManager_ = nullptr;

	// ゲームスタートフラグ
	bool isGameStart_ = false;
};


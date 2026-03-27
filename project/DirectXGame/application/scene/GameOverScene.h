#pragma once
#include <BaseScene.h>
#include <Sprite.h>
#include <FadeManager.h>
#include <Object3d.h>
#include <CameraManager.h>
#include <Input.h>
#include <Player.h>
#include <PostEffectManager.h>
#include <ParticleManager.h>
#include <ParticleEmitter.h>

/// 調整用定数（マジックナンバー排除）
namespace GameOverDefaults {
	// カメラ
	inline constexpr Vector3 kCamPos{ 0.0f, 1.0f, -10.0f };
	inline constexpr Vector3 kCamRot{ 0.1f, 0.0f, 0.0f };

	// テキスト
	inline constexpr Vector3 kTextTranslate{ 0.0f, 1.4f, 0.0f };
	inline constexpr Vector3 kTextRotate{ 0.0f, 1.6f, 0.0f };

	// ガイド
	inline constexpr Vector3 kGuideTranslate{ 0.0f, -3.0f, 4.0f };
	inline constexpr Vector3 kGuideRotate{ -1.0f, 0.0f, 0.0f };

	// リトライテキスト
	inline constexpr Vector3 kRetryTextTranslate{ 0.0f, -1.6f, 4.0f };
	inline constexpr Vector3 kRetryTextRotate{ -1.0f, 0.0f, 0.0f };

	// プレイヤー落下演出
	inline constexpr Vector3 kPlayerInitTranslate{ 0.0f, 3.0f, 0.0f };
	inline constexpr Vector3 kPlayerInitRotate{ 0.0f, 0.0f, -1.0f };
	inline constexpr float   kPlayerFallSpeed = 0.01f;
	inline constexpr float   kPlayerRotateSpeedX = 0.02f;

	// フェード
	inline constexpr float kFadeStep = 0.02f;

	inline constexpr float kDeltaTime60Hz = 1.0f / 60.0f;
}

struct particleParameters {
	std::string textureName = "resources/fog.png";
	uint32_t rate = 60;
	uint32_t count = 3;
};
/// <summary>
/// ゲームオーバーシーン
/// </summary>
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

	// 入力処理
	void UpdateInput();

private:
	// ステートマシン
	enum class MenuSelect {
		Restart,
		ReturnToTitle,
	};

	MenuSelect menuSelect_ = MenuSelect::Restart;
private:
	// 入力の初期化
	std::unique_ptr<Input> input_ = nullptr;
	
	// フェードマネージャー
	std::unique_ptr<FadeManager> fadeManager_;

	// オブジェクト3D
	std::unique_ptr<Object3d> gameOverText_;

	// 3Dワールド変換
	WorldTransform gameOverTextTransform_;

	// ガイドオブジェクト
	std::unique_ptr<Object3d> gameOverGuide_;

	// ガイドワールド変換
	WorldTransform gameOverGuideTransform_;

	// リトライテキスト
	std::unique_ptr<Object3d> retryText_;

	// リトライテキストワールド変換
	WorldTransform retryTextTransform_;

	// カメラマネージャー
	std::unique_ptr<CameraManager> cameraManager_;

	// skydome
	std::unique_ptr<Object3d> skydome_ = nullptr;

	// skydomeワールド変換
	WorldTransform skydomeTransform_;

	// フェード開始フラグ
	bool fadeStarted_ = false;

	// タイトルに戻るフラグ
	bool returnToTitle_ = false;

	// プレイヤーオブジェクト
	std::unique_ptr<Object3d> player_ = nullptr;

	// プレイヤーワールド変換
	WorldTransform playerTransform_;

	// パーティクルマネージャー
	ParticleManager* particleManager_ = nullptr;
	// 煙エフェクト
	std::unique_ptr<ParticleEmitter> smokeEffect_ = nullptr;

	particleParameters parameters_;

	Vector3 smokeVelocity_ = { 0.0f, 5.0f, 0.0f };
};


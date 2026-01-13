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
#include <CameraState.h>

/// 調整用定数（マジックナンバー排除）
namespace GamePlayDefaults {
	// スプライト座標
	inline constexpr Vector2 kSpritePos{ 100.0f, 100.0f };

	// スタート演出
	inline constexpr bool  kStartCameraEasing = true;
	inline constexpr float kStartCameraDurationSec = 5.0f;
	inline constexpr Vector3 kStartCamPos{ 70.0f, -10.0f, -20.0f };
	inline constexpr Vector3 kStartCamRot{ 0.0f,  0.0f,   0.0f };
	inline constexpr Vector3 kEndCamPos{   0.0f,   1.0f, -10.0f };
	inline constexpr Vector3 kEndCamRot{   0.1f,   0.0f,   0.0f };

	// カーブ移動
	inline constexpr float kCurveSpeed = 0.004f;

	// ゲームオーバー
	inline constexpr float kGameOverTimerSec = 2.0f;

	// ゲームクリア
	inline constexpr float kGameClearMoveDurationSec = 2.0f;
	inline constexpr float kGameClearPlayerLaunchSpeed = 16.0f;

	// セグメント
	inline constexpr float kSegmentDefaultDurationSec = 1.0f;

	// 時間ステップ
	inline constexpr float kDeltaTime60Hz = 1.0f / 60.0f;
}

/// <summary>
/// ゲームプレイシーン
/// </summary>
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

	// JSONから読み込んだオブジェクトのImGui調整
	void DrawImGuiImportObjectsFromJson();

	// スタートカメライージング更新
	//void UpdateStartCameraEasing();

public:
	// ステート
	enum class GameSceneState {
		Start,
		Play,
		GameClear,
		GameOver,
		Pause
	};

	// カメラモード
	enum class CameraMode {
		Free,
		FollowPlayer,
		DynamicFollow,
		CenterPlayer
	};

	// ステート遷移関数
	void TransitionToGameplayState();
	void TransitionToClearHoldState();
	void TransitionToClearZoomState();
	void TransitionToClearFlyAwayState();

	// ゲームクリア通知
	void OnGameClear();

	// アクセサー
	CameraManager* GetCameraManager() { return cameraManager_.get(); }
	Player* GetPlayer() { return player_.get(); }
	std::vector<std::unique_ptr<Enemy>>& GetEnemies() { return enemies_; }
	const LevelData* GetLevelData() const { return levelData_; }
	const std::vector<Vector3>& GetCurvePoints() const { return curvePoints_; }
	Vector3 GetStartCameraPos() const { return startCameraPos_; }
	Vector3 GetEndCameraPos() const { return endCameraPos_; }
	Vector3 GetStartCameraRot() const { return startCameraRot_; }
	Vector3 GetEndCameraRot() const { return endCameraRot_; }
	float GetStartCameraDuration() const { return kStartCameraDuration_; }
	float GetGameClearMoveDuration() const { return kGameClearMoveDuration_; }
	float GetGameClearPlayerLaunchSpeed() const { return kGameClearPlayerLaunchSpeed_; }
	bool IsGameClearFadeStarted() const { return gameClearFadeStarted_; }

	void SetCameraMode(CameraMode mode) { cameraMode_ = mode; }
	void ShowGameClearText();
	void HideGameClearText();
	void StartGameClearFade();
	void RestrictPlayerInsideCameraView();

private:
	
	void CheckAllCollisions();
	void LoadLevel(const LevelData* levelData);
	void UpdateGameClear();
	bool IsInCameraView(const Vector3& worldPos);

	// カメラステート管理
	std::unique_ptr<CameraState> currentCameraState_;
	void ChangeState(std::unique_ptr<CameraState> newState);

	// スプライト
	std::unique_ptr<Sprite> sprite_;

	// 当たり判定マネージャ
	std::unique_ptr<CollisionManager> collisionManager_;

	// 入力
	Input* input_ = nullptr;

	// スプライトの座標
	Vector2 spritePosition_ = GamePlayDefaults::kSpritePos;

	// オーディオ
	SoundData soundData1_;

	// プレイヤー
	std::unique_ptr<Player> player_ = nullptr;

	// プレイヤーの弾
	std::list<std::unique_ptr<PlayerBullet>>* playerBullets_;

	// プレイヤーのチャージ弾
	std::list<std::unique_ptr<PlayerChargeBullet>>* playerChargeBullets_;

	// 敵の弾
	std::list<std::unique_ptr<EnemyBullet>>* enemyBullets_;

	// 敵
	std::unique_ptr<Enemy> enemy_ = nullptr;

	// レベルデータ
	LevelData* levelData_ = nullptr;

	// 複数のモデルを管理するためのコンテナ
	std::unordered_map<std::string, std::unique_ptr<Model>> models_;

	// 複数のオブジェクトを管理するためのコンテナ
	std::vector<std::unique_ptr<Object3d>> objects_;

	// スカイボックス
	std::unique_ptr<Skybox> skybox_ = nullptr;

	// フェード演出管理
	std::unique_ptr<FadeManager> fadeManager_;

	// 複数の敵を管理するためのコンテナ
	std::vector<std::unique_ptr<Enemy>> enemies_;

	// カメラマネージャー
	std::unique_ptr<CameraManager> cameraManager_;
	CameraMode cameraMode_ = CameraMode::Free;

	// スタート演出用カメラ
	Vector3 startCameraPos_ = { 0.0f, 1.0f, -25.0f };
	Vector3 endCameraPos_ = { 0.0f, 1.0f, -10.0f };
	Vector3 startCameraRot_ = { 0.1f, 0.0f, 0.0f };
	Vector3 endCameraRot_ = { 0.1f, 0.0f, 0.0f };
	const float kStartCameraDuration_ = 3.0f;

	// カーブ追従
	std::vector<Vector3> curvePoints_;
	const float kDeltaTime_ = 1.0f / 60.0f;

	// ゲームオーバー
	bool isGameOver_ = false;
	float gameOverTimer_ = 0.0f;
	bool fadeStarted_ = false;

	// ゲームクリア
	bool isGameClear_ = false;
	bool isEnd_ = false;

	// タイトルに戻るテキスト
	std::unique_ptr<Object3d> backToTitle_;

	// タイトルに戻るテキストのワールド変換
	WorldTransform textTitle_;

	// ゲームクリアテキスト
	std::unique_ptr<Object3d> gameClearText_;
	WorldTransform gameClearTextTransform_;
	std::unique_ptr<Object3d> pressSpaceKeyText_;
	WorldTransform pressSpaceKeyTransform_;
	bool gameClearTextVisible_ = false;

	// ゲームクリア演出
	const float kGameClearMoveDuration_ = 2.0f;
	const float kGameClearPlayerLaunchSpeed_ = 5.0f;
	bool gameClearFadeStarted_ = false;
};


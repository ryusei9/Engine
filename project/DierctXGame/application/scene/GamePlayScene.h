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
	void UpdateStartCameraEasing();

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

	template<typename T>
	T MyMin(const T& a, const T& b) {
		return (a < b) ? a : b;
	}

	template<typename T>
	T MyMax(const T& a, const T& b) {
		return (a > b) ? a : b;
	}

private:
	// 衝突判定と応答
	void CheckAllCollisions();

	// カメラルートの読み込み
	void LoadLevel(const LevelData* levelData);

	// カーブに沿ってプレイヤーを移動させる
	void UpdateCameraOnCurve();

	// プレイヤーがカメラの視界内に収まるように制限する
	void RestrictPlayerInsideCameraView();

	// プレイヤーがカメラに追従する
	void UpdatePlayerFollowCamera();

	// カメラの中に入っているか
	bool IsInCameraView(const Vector3& worldPos);

	// ゲームクリアの演出更新
	void UpdateGameClear();

	// スプライトコモン
	SpriteCommon* spriteCommon_ = nullptr;

	// ダイレクトXコモン
	DirectXCommon* directXCommon_ = nullptr;

	// WinApp
	WinApp* winApp_ = nullptr;

	// 当たり判定マネージャ
	std::unique_ptr<CollisionManager> collisionManager_;

	// スプライト
	std::unique_ptr<Sprite> sprite_ = nullptr;

	// 入力の初期化
	Input* input_ = nullptr;

	// スプライトの座標
	Vector2 spritePosition_ = { 100.0f, 100.0f };

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

	// カメラモードの状態
	CameraMode cameraMode_ = CameraMode::DynamicFollow;

	// --- スタート演出用 ---
	bool isStartCameraEasing_ = true;
	float startCameraTimer_ = 0.0f;
	const float kStartCameraDuration_ = 5.0f;
	Vector3 startCameraPos_ = { 70.0f, -10.0f, -20.0f };
	Vector3 startCameraRot_ = { 0.0f, 0.0f, 0.0f };
	Vector3 endCameraPos_ = { 0.0f, 1.0f, -10.0f };
	Vector3 endCameraRot_ = { 0.1f, 0.0f, 0.0f };

	// タイトルに戻るテキスト
	std::unique_ptr<Object3d> backToTitle_;

	// タイトルに戻るテキストのワールド変換
	WorldTransform textTitle_;

	// カーブ座標リスト
	std::vector<Vector3> curvePoints_;

	// カーブ進行度（0.0～1.0）
	float curveProgress_ = 0.0f;

	// カーブの現在インデックス
	size_t curveIndex_ = 0;

	// カメラ移動速度（1フレームあたりの進行度）
	float curveSpeed_ = 0.004f;

	// ゲームオーバー処理用タイマー
	float gameOverTimer_ = 2.0f;

	// ゲームオーバーフラグ
	bool isGameOver_ = false;

	// フェード開始フラグ
	bool fadeStarted_ = false;

	// カメラ
	std::unique_ptr<Camera> camera_;

	// ゲーム終了フラグ
	bool isEnd_ = false;

	// 仮ゲームクリア
	bool isGameClear_ = false;

	// ゲームクリア時のカメラ遷移・発射制御用
	// 待機フェーズ
	bool gameClearCameraWaiting_ = false;

	// 待機タイマー
	float gameClearWaitTimer_ = 0.0f;

	// カメラ移動フェーズ
	bool gameClearCameraMoving_ = false;

	// カメラ移動タイマー
	float gameClearMoveTimer_ = 0.0f;

	// カメラ移動時間
	const float kGameClearMoveDuration_ = 2.0f;

	// カメラ開始位置と目標位置
	Vector3 gameClearCamStartPos_ = { 0.0f, 0.0f, 0.0f };
	Vector3 gameClearCamTargetPos_ = { 0.0f, 0.0f, 0.0f };

	// プレイヤー発射フェーズ
	bool gameClearPlayerLaunched_ = false;

	// フェード開始フラグ
	bool gameClearFadeStarted_ = false;

	// プレイヤー発射速度（単位: ワールド単位 / 秒）
	const float kGameClearPlayerLaunchSpeed_ = 16.0f;

	// ゲームクリア演出用テキスト
	std::unique_ptr<Object3d> gameClearText_;

	// スペースキーを押してねテキスト
	std::unique_ptr<Object3d> pressSpaceKeyText_;

	// ゲームクリア演出用テキストの表示フラグ
	bool gameClearTextVisible_ = false;

	// ゲームクリア演出用テキストのワールド変換
	WorldTransform gameClearTextTransform_;

	// スペースキーを押してねテキストのワールド変換
	WorldTransform pressSpaceKeyTransform_;

	// カーブセグメントタイマー
	float segmentTimer_ = 0.0f;

	// 現在のセグメントの所要時間
	float currentSegmentDuration_ = 1.0f;

	// デルタタイム
	const float kDeltaTime_ = 1.0f / 60.0f;
};


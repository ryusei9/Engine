#pragma once
#include "Vector3.h"
#include <vector>

class GamePlayScene;
class CameraManager;

// カメラステート基底クラス
class CameraState {
public:
	virtual ~CameraState() = default;
	
	// ステート開始時の初期化
	virtual void Enter(GamePlayScene* scene) = 0;
	
	// 毎フレーム更新
	virtual void Update(GamePlayScene* scene, float deltaTime) = 0;
	
	// ステート終了時の処理
	virtual void Exit(GamePlayScene* scene) = 0;
};

// ゲーム開始演出ステート (Intro_Pan)
class CameraStateIntroPan : public CameraState {
public:
	void Enter(GamePlayScene* scene) override;
	void Update(GamePlayScene* scene, float deltaTime) override;
	void Exit(GamePlayScene* scene) override;

private:
	float timer_ = 0.0f;
	Vector3 startPos_;
	Vector3 endPos_;
	Vector3 startRot_;
	Vector3 endRot_;
	float duration_ = 0.0f;
};

// 通常プレイステート (Gameplay)
class CameraStateGameplay : public CameraState {
public:
	void Enter(GamePlayScene* scene) override;
	void Update(GamePlayScene* scene, float deltaTime) override;
	void Exit(GamePlayScene* scene) override;

private:
	std::vector<Vector3> curvePoints_;
	size_t curveIndex_ = 0;
	float segmentTimer_ = 0.0f;
};

// クリア時プレイヤーホールドステート (Clear_Hold)
class CameraStateClearHold : public CameraState {
public:
	void Enter(GamePlayScene* scene) override;
	void Update(GamePlayScene* scene, float deltaTime) override;
	void Exit(GamePlayScene* scene) override;

private:
	float waitTimer_ = 0.0f;
	float timer_ = 0.0f;
	Vector3 startPos_;
	Vector3 targetPos_;
	float moveDuration_ = 0.0f;
};

// クリア時ズームステート (Clear_Zoom)
class CameraStateClearZoom : public CameraState {
public:
	void Enter(GamePlayScene* scene) override;
	void Update(GamePlayScene* scene, float deltaTime) override;
	void Exit(GamePlayScene* scene) override;
};

// クリア時退場演出ステート (Clear_FlyAway)
class CameraStateClearFlyAway : public CameraState {
public:
	void Enter(GamePlayScene* scene) override;
	void Update(GamePlayScene* scene, float deltaTime) override;
	void Exit(GamePlayScene* scene) override;

private:
	float fadeDelayTimer_ = 0.0f;
	bool fadeTimerInitialized_ = false;
};
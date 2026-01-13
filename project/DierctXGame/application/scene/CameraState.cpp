#include "CameraState.h"
#include "GamePlayScene.h"
#include "CameraManager.h"
#include "Player.h"
#include "Enemy.h"
#include "Input.h"
#include "Object3dCommon.h"
#include "Multiply.h"
#include "Inverse.h"
#include <algorithm>
#undef min
#undef max

// ========== CameraStateIntroPan ==========
void CameraStateIntroPan::Enter(GamePlayScene* scene) {
	timer_ = 0.0f;
	startPos_ = scene->GetStartCameraPos();
	endPos_ = scene->GetEndCameraPos();
	startRot_ = scene->GetStartCameraRot();
	endRot_ = scene->GetEndCameraRot();
	duration_ = scene->GetStartCameraDuration();
	
	scene->GetCameraManager()->SetCameraPosition(startPos_);
	scene->GetCameraManager()->SetCameraRotation(startRot_);
	scene->GetPlayer()->SetPlayerControlEnabled(false);
}

void CameraStateIntroPan::Update(GamePlayScene* scene, float deltaTime) {
	timer_ += deltaTime;
	float t = std::clamp(timer_ / duration_, 0.0f, 1.0f);

	// easeInOutCubic
	float easeT;
	if (t < 0.5f) {
		easeT = 4.0f * t * t * t;
	} else {
		float p = (t - 1.0f);
		easeT = 1.0f + 4.0f * p * p * p;
	}

	Vector3 pos = {
		std::lerp(startPos_.x, endPos_.x, easeT),
		std::lerp(startPos_.y, endPos_.y, easeT),
		std::lerp(startPos_.z, endPos_.z, easeT)
	};
	Vector3 rot = {
		std::lerp(startRot_.x, endRot_.x, easeT),
		std::lerp(startRot_.y, endRot_.y, easeT),
		std::lerp(startRot_.z, endRot_.z, easeT)
	};

	scene->GetCameraManager()->SetCameraPosition(pos);
	scene->GetCameraManager()->SetCameraRotation(rot);

	if (t >= 1.0f) {
		scene->TransitionToGameplayState();
	}
}

void CameraStateIntroPan::Exit(GamePlayScene* scene) {
	scene->GetPlayer()->SetPlayerControlEnabled(true);
	for (auto& enemy : scene->GetEnemies()) {
		enemy->SetControlEnabled(true);
	}
}

// ========== CameraStateGameplay ==========
void CameraStateGameplay::Enter(GamePlayScene* scene) {
	curvePoints_ = scene->GetCurvePoints();
	curveIndex_ = 0;
	segmentTimer_ = 0.0f;
	scene->SetCameraMode(GamePlayScene::CameraMode::Free);
}

void CameraStateGameplay::Update(GamePlayScene* scene, float deltaTime) {
	// カーブに沿ってカメラを移動
	if (curvePoints_.size() < 2) return;

	if (curveIndex_ >= curvePoints_.size() - 1) {
		scene->GetCameraManager()->SetCameraPosition(curvePoints_.back());
		scene->OnGameClear();
		return;
	}

	const auto& levelData = scene->GetLevelData();
	float duration = levelData->curves[0].times[curveIndex_ + 1];
	if (curveIndex_ == 0) duration = levelData->curves[0].times[1];

	segmentTimer_ += deltaTime;
	float t = std::clamp(segmentTimer_ / duration, 0.0f, 1.0f);

	// Catmull-Rom補間
	size_t p0 = std::max(static_cast<int32_t>(curveIndex_) - 1, 0);
	size_t p1 = curveIndex_;
	size_t p2 = curveIndex_ + 1;
	size_t p3 = std::min(static_cast<int32_t>(curvePoints_.size()) - 1, static_cast<int32_t>(curveIndex_) + 2);

	auto catmullRom = [&](const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d, float t) {
		float t2 = t * t;
		float t3 = t2 * t;
		return Vector3(
			0.5f * (2.0f * b.x + (-a.x + c.x) * t + (2.0f * a.x - 5.0f * b.x + 4.0f * c.x - d.x) * t2 + (-a.x + 3.0f * b.x - 3.0f * c.x + d.x) * t3),
			0.5f * (2.0f * b.y + (-a.y + c.y) * t + (2.0f * a.y - 5.0f * b.y + 4.0f * c.y - d.y) * t2 + (-a.y + 3.0f * b.y - 3.0f * c.y + d.y) * t3),
			0.5f * (2.0f * b.z + (-a.z + c.z) * t + (2.0f * a.z - 5.0f * b.z + 4.0f * c.z - d.z) * t2 + (-a.z + 3.0f * b.z - 3.0f * c.z + d.z) * t3)
		);
	};

	Vector3 newPos = catmullRom(curvePoints_[p0], curvePoints_[p1], curvePoints_[p2], curvePoints_[p3], t);
	scene->GetCameraManager()->SetCameraPosition(newPos);

	if (t >= 1.0f) {
		curveIndex_++;
		segmentTimer_ = 0.0f;
	}

	// プレイヤー追従処理
	Vector3 prevCamPos = scene->GetCameraManager()->GetMainCamera()->GetTranslate();
	Camera* cam = scene->GetCameraManager()->GetMainCamera();
	Vector3 camPos = cam->GetTranslate();
	Vector3 camMove = { camPos.x - prevCamPos.x, camPos.y - prevCamPos.y, camPos.z - prevCamPos.z };

	Player* player = scene->GetPlayer();
	Vector3 playerPos = player->GetPosition();
	playerPos.x += camMove.x;
	playerPos.y += camMove.y;
	playerPos.z = camPos.z + 10.0f;
	player->SetPosition(playerPos);

	// プレイヤーの画面内制限
	scene->RestrictPlayerInsideCameraView();
}

void CameraStateGameplay::Exit(GamePlayScene* scene) {
}

// ========== CameraStateClearHold ==========
void CameraStateClearHold::Enter(GamePlayScene* scene) {
	waitTimer_ = GamePlayDefaults::kDeltaTime60Hz * 120.0f; // 2秒待機
	timer_ = 0.0f;
	moveDuration_ = scene->GetGameClearMoveDuration();
	scene->GetPlayer()->SetPlayerControlEnabled(false);
}

void CameraStateClearHold::Update(GamePlayScene* scene, float deltaTime) {
	if (waitTimer_ > 0.0f) {
		waitTimer_ -= deltaTime;
		return;
	}

	// 初回のみ開始位置と目標位置を設定
	if (timer_ == 0.0f) {
		startPos_ = scene->GetCameraManager()->GetMainCamera()->GetTranslate();
		targetPos_ = scene->GetPlayer()->GetPosition() + Vector3{ 0.0f, 0.0f, -5.0f };
	}

	timer_ += deltaTime;
	float t = std::clamp(timer_ / moveDuration_, 0.0f, 1.0f);

	// easeInOutCubic
	float easeT;
	if (t < 0.5f) {
		easeT = 4.0f * t * t * t;
	} else {
		float p = (t - 1.0f);
		easeT = 1.0f + 4.0f * p * p * p;
	}

	Vector3 pos = {
		std::lerp(startPos_.x, targetPos_.x, easeT),
		std::lerp(startPos_.y, targetPos_.y, easeT),
		std::lerp(startPos_.z, targetPos_.z, easeT)
	};

	scene->SetCameraMode(GamePlayScene::CameraMode::CenterPlayer);
	scene->GetCameraManager()->SetCameraPosition(pos);

	if (t >= 1.0f) {
		scene->TransitionToClearZoomState();
	}
}

void CameraStateClearHold::Exit(GamePlayScene* scene) {
	scene->GetCameraManager()->SetCameraPosition(targetPos_);
}

// ========== CameraStateClearZoom ==========
void CameraStateClearZoom::Enter(GamePlayScene* scene) {
	scene->ShowGameClearText();
}

void CameraStateClearZoom::Update(GamePlayScene* scene, float deltaTime) {
	// スペースキー待ち
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		scene->TransitionToClearFlyAwayState();
	}
}

void CameraStateClearZoom::Exit(GamePlayScene* scene) {
	scene->HideGameClearText();
}

// ========== CameraStateClearFlyAway ==========
void CameraStateClearFlyAway::Enter(GamePlayScene* scene) {
	fadeDelayTimer_ = 1.0f;
	fadeTimerInitialized_ = true;
	scene->SetCameraMode(GamePlayScene::CameraMode::DynamicFollow);
}

void CameraStateClearFlyAway::Update(GamePlayScene* scene, float deltaTime) {
	// プレイヤーを飛ばす
	Player* player = scene->GetPlayer();
	Vector3 pos = player->GetPosition();
	pos.x += scene->GetGameClearPlayerLaunchSpeed() * deltaTime;
	player->SetPosition(pos);

	// フェード遅延
	if (fadeTimerInitialized_) {
		fadeDelayTimer_ -= deltaTime;
		if (fadeDelayTimer_ <= 0.0f && !scene->IsGameClearFadeStarted()) {
			scene->StartGameClearFade();
		}
	}
}

void CameraStateClearFlyAway::Exit(GamePlayScene* scene) {
}
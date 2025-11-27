#pragma once
#include "WorldTransform.h"
#include <ImGui.h>
#include "Camera.h"
class CameraManager
{
public:
	// 初期化
	void Initialize(Camera* camera);

	// 更新
	void Update();

	// ImGui描画
	void DrawImGui();

	// 対象とカメラを一緒に移動させる
	void MoveTargetAndCamera(WorldTransform target, const Vector3& delta);

	// カメラが対象に向き続ける
	// 既定では即時回転。eased=true にするとイージングで回転する（easeFactor は補間係数 0..1）
	void LookAtTarget(const Vector3& targetPosition, bool eased = false, float easeFactor = 0.1f);

	// セッター
	void SetMainCamera(Camera* camera) { mainCamera_ = camera; }

	void SetCameraPosition(const Vector3& position) {
		if (mainCamera_) {
			mainCamera_->SetTranslate(position);
		}
	}

	void SetCameraRotation(const Vector3& rotation) {
		if (mainCamera_) {
			mainCamera_->SetRotate(rotation);
		}
	}

	void SetYPosition(float x) {
		if (mainCamera_) {
			mainCamera_->SetXPosition(x);
		}
	}

	Camera* GetMainCamera() const { return mainCamera_; }

	
private:
	Camera* mainCamera_ = nullptr;

	// イージング用
	bool isEasing_ = false;
	Vector3 easeTargetRotation_ = {0.0f, 0.0f, 0.0f}; // 目標回転（ラジアン）
	float easeFactor_ = 0.1f;                          // 補間係数（0..1、1で即時）
	float angleEpsilon_ = 1e-3f;                       // しきい値（ラジアン）
};


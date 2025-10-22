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
	void LookAtTarget(const Vector3& targetPosition);

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
};


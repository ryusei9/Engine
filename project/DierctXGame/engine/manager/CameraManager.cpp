#include "CameraManager.h"
#include "Camera.h"
#include "Transform.h"
#include "Object3dCommon.h"
#include <cmath> // 追加: atan2f, sqrtf, fmod 等

namespace {
	const float kPI = 3.14159265358979323846f;
	// 角度を (-PI, PI] に正規化
	static float NormalizeAngle(float a) {
		while (a > kPI) a -= 2.0f * kPI;
		while (a <= -kPI) a += 2.0f * kPI;
		return a;
	}
	// current から target への最短角度差（target - current）を返す
	static float ShortestAngleDiff(float current, float target) {
		float diff = NormalizeAngle(target - current);
		return diff;
	}
}

void CameraManager::Initialize(Camera* camera)
{
    mainCamera_ = camera;
}

void CameraManager::Update()
{
	// イージング処理（有効なら毎フレーム少しずつ回転を更新）
	if (mainCamera_ && isEasing_) {
		const Vector3 curRot = mainCamera_->GetRotate();
		// 現在のピッチ（X軸回転）とヨー（Y軸回転）
		float curPitch = curRot.x;
		float curYaw = curRot.y;

		// 目標との差を最短で取り、補間
		float diffPitch = ShortestAngleDiff(curPitch, easeTargetRotation_.x);
		float diffYaw = ShortestAngleDiff(curYaw, easeTargetRotation_.y);

		// 新しい回転（線形補間）
		float newPitch = curPitch + diffPitch * easeFactor_;
		float newYaw = curYaw + diffYaw * easeFactor_;

		// 小さくなったら完了フラグを下ろす
		if (std::fabs(diffPitch) < angleEpsilon_ && std::fabs(diffYaw) < angleEpsilon_) {
			isEasing_ = false;
			newPitch = easeTargetRotation_.x;
			newYaw = easeTargetRotation_.y;
		}

		mainCamera_->SetRotate(Vector3(newPitch, newYaw, 0.0f));
	}

	mainCamera_->Update();
	//Object3dCommon::GetInstance()->SetDefaultCamera(mainCamera_);
}

void CameraManager::DrawImGui() {
#ifdef USE_IMGUI
    ImGui::Begin("Camera Manager");
    Vector3 camPos = mainCamera_->GetTranslate();
    Vector3 camRot = mainCamera_->GetRotate();
	float camFovY = mainCamera_->GetFovY();
	Matrix4x4 camWorldMat = mainCamera_->GetWorldMatrix();
	Matrix4x4 camViewMat = mainCamera_->GetViewMatrix();
	Matrix4x4 camProjMat = mainCamera_->GetProjectionMatrix();
	Matrix4x4 camViewProjMat = mainCamera_->GetViewProjectionMatrix();
    if (ImGui::DragFloat3("Camera Position", &camPos.x)) {
        mainCamera_->SetTranslate(camPos);
    }
    if (ImGui::DragFloat3("Camera Rotation", &camRot.x)) {
        mainCamera_->SetRotate(camRot);
    }
	if (ImGui::DragFloat("Camera FovY", &camFovY, 0.01f, 0.1f, 3.14f)) {
		mainCamera_->SetFovY(camFovY);
	}
	ImGui::Text("Camera World Matrix:");
	for (int i = 0; i < 4; ++i) {
		ImGui::Text("| %.2f %.2f %.2f %.2f |", camWorldMat.m[i][0], camWorldMat.m[i][1], camWorldMat.m[i][2], camWorldMat.m[i][3]);
	}
	ImGui::Text("Camera View Matrix:");
	for (int i = 0; i < 4; ++i) {
		ImGui::Text("| %.2f %.2f %.2f %.2f |", camViewMat.m[i][0], camViewMat.m[i][1], camViewMat.m[i][2], camViewMat.m[i][3]);
	}
	ImGui::Text("Camera Projection Matrix:");
	for (int i = 0; i < 4; ++i) {
		ImGui::Text("| %.2f %.2f %.2f %.2f |", camProjMat.m[i][0], camProjMat.m[i][1], camProjMat.m[i][2], camProjMat.m[i][3]);
	}
	ImGui::Text("Camera View-Projection Matrix:");
	for (int i = 0; i < 4; ++i) {
		ImGui::Text("| %.2f %.2f %.2f %.2f |", camViewProjMat.m[i][0], camViewProjMat.m[i][1], camViewProjMat.m[i][2], camViewProjMat.m[i][3]);
	}
    ImGui::End();
#endif
}

void CameraManager::MoveTargetAndCamera(WorldTransform target, const Vector3& delta) {
    
    target.translate_ += delta;
    
    if (mainCamera_) {
        //Vector3 camPos = mainCamera_->GetTranslate();
        mainCamera_->SetXPosition(mainCamera_->GetTranslate().x + delta.x);
    }
}

void CameraManager::LookAtTarget(const Vector3& targetPosition, bool eased, float easeFactor) {
    if (!mainCamera_) return;

    // カメラ位置と方向ベクトル
    Vector3 camPos = mainCamera_->GetTranslate();
    Vector3 direction = targetPosition - camPos;

    // 方向ベクトルから回転角を計算（ピッチとヨー）
    float yaw = atan2f(direction.x, direction.z);
    float pitch = -atan2f(direction.y, sqrtf(direction.x * direction.x + direction.z * direction.z));

    if (!eased) {
        // 即時回転（従来通り）
        mainCamera_->SetRotate(Vector3(pitch, yaw, 0.0f));
        isEasing_ = false;
    }
    else {
        // イージング開始：目標回転を設定してフラグオン
        easeTargetRotation_ = Vector3(pitch, yaw, 0.0f);
        easeFactor_ = easeFactor;
        // clamp easeFactor (安全対策)
        if (easeFactor_ < 0.0f) easeFactor_ = 0.0f;
        if (easeFactor_ > 1.0f) easeFactor_ = 1.0f;
        isEasing_ = true;
    }
}


#include "CameraManager.h"
#include "Camera.h"
#include "Transform.h"
#include "Object3dCommon.h"

void CameraManager::Initialize(Camera* camera)
{
    mainCamera_ = camera;
}

void CameraManager::Update()
{
	mainCamera_->Update();
	//Object3dCommon::GetInstance()->SetDefaultCamera(mainCamera_);
}

void CameraManager::DrawImGui() {
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
}

void CameraManager::MoveTargetAndCamera(WorldTransform target, const Vector3& delta) {
    
    target.translate_ += delta;
    
    if (mainCamera_) {
        //Vector3 camPos = mainCamera_->GetTranslate();
        mainCamera_->SetXPosition(mainCamera_->GetTranslate().x + delta.x);
    }
}

void CameraManager::LookAtTarget(const Vector3& targetPosition) {
    if (!mainCamera_) return;
    Vector3 camPos = mainCamera_->GetTranslate();
    Vector3 direction = targetPosition - camPos;
    // 方向ベクトルから回転角を計算（例: Y軸回転のみ考慮）
    float yaw = atan2f(direction.x, direction.z);
    float pitch = -atan2f(direction.y, sqrtf(direction.x * direction.x + direction.z * direction.z));
    mainCamera_->SetRotate(Vector3(pitch, yaw, 0.0f));
}

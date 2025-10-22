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
    if (ImGui::DragFloat3("Camera Position", &camPos.x)) {
        mainCamera_->SetTranslate(camPos);
    }
    if (ImGui::DragFloat3("Camera Rotation", &camRot.x)) {
        mainCamera_->SetRotate(camRot);
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

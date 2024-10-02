#define NOMINMAX
#include "CameraController.h"
#include "Player.h"

void CameraController::initialize(ViewProjection* viewProjection) { 
	viewProjection_ = viewProjection;

}

void CameraController::Update() {
//	// 追従対象のワールドトランスフォームを参照
//	/*const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
//	const Vector3& targetVelocity = target_->GetVelocity();*/
//	// 追従対象とオフセットと追従対象の速度からカメラの座標を計算
//	targetPos.x = targetWorldTransform.translation_.x + targetOffset_.x + targetVelocity.x * kVelocityBias;
//	targetPos.y = targetWorldTransform.translation_.y + targetOffset_.y + targetVelocity.y * kVelocityBias;
//	targetPos.z = targetWorldTransform.translation_.z + targetOffset_.z + targetVelocity.z * kVelocityBias;
//
//	// 座標補間によりゆったり追従
//	viewProjection_->translation_ = mathMatrix_->lerp(viewProjection_->translation_, targetPos, kInterpolationRate);
//
//	// 追従対象が画面外に出ないように補正
//	viewProjection_->translation_.x = std::max(viewProjection_->translation_.x,targetPos.x + margin.left);
//	viewProjection_->translation_.x = std::min(viewProjection_->translation_.x,targetPos.x + margin.right);
//	viewProjection_->translation_.y = std::max(viewProjection_->translation_.y,targetPos.y + margin.bottom);
//	viewProjection_->translation_.y = std::min(viewProjection_->translation_.y,targetPos.y + margin.top);
//
//	// 移動範囲制限
//	viewProjection_->translation_.x = std::max(viewProjection_->translation_.x, movebleArea_.left);
//	viewProjection_->translation_.x = std::min(viewProjection_->translation_.x, movebleArea_.right);
//	viewProjection_->translation_.y = std::max(viewProjection_->translation_.y, movebleArea_.bottom);
//	viewProjection_->translation_.y = std::min(viewProjection_->translation_.y, movebleArea_.top);
//	// 行列を更新
//	viewProjection_->UpdateMatrix();
//}
//
//void CameraController::Reset() {
//	// 追従対象のワールドトランスフォームを参照
//	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
//	// 追従対象とオフセットからカメラの座標を計算
//	viewProjection_->translation_.x = targetWorldTransform.translation_.x + targetOffset_.x;
//	viewProjection_->translation_.y = targetWorldTransform.translation_.y + targetOffset_.y;
//	viewProjection_->translation_.z = targetWorldTransform.translation_.z + targetOffset_.z;
}

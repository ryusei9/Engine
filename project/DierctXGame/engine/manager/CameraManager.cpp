#include "CameraManager.h"
#include "Camera.h"
#include "Transform.h"
#include "Object3dCommon.h"
#include <cmath> // 追加: atan2f, sqrtf, fmod 等

/*
 CameraManager.cpp

 概要:
 - カメラの共有管理と「イージング（滑らかな回転遷移）」「ターゲットへの注視（LookAt）」など
   カメラ制御に関する補助的な機能を提供するモジュール。
 - シーン内で利用する Camera* を保持し、毎フレーム Update() で必要な補間処理を実行する。
 - 主な責務:
   * メインカメラの登録と毎フレーム更新
   * カメラ回転のイージング（easeTargetRotation_, easeFactor_ を用いた線形補間）
   * 指定座標へカメラを向ける LookAtTarget の計算（即時 or イージング）
   * カメラの簡易移動補助 MoveTargetAndCamera

 注意点:
 - 本クラスはスレッド非対応。メインスレッドから呼び出すことを想定。
 - 角度（ラジアン）の扱いは -π..π に正規化して最短経路で回転補間する。
 - easeFactor_ は 0..1 の範囲を期待する（範囲外はクランプする実装あり）。
 - LookAtTarget の pitch/yaw 計算は典型的な球面座標変換を利用している（gimbal lock の簡易対策は入れていない）。
*/

namespace {
	const float kPI = 3.14159265358979323846f;

	// 角度を (-PI, PI] の範囲に正規化する
	static float NormalizeAngle(float a) {
		while (a > kPI) a -= 2.0f * kPI;
		while (a <= -kPI) a += 2.0f * kPI;
		return a;
	}

	// 現在角 current から target への最短角度差（target - current）を返す
	// 返り値は正負付きの差分（ラジアン）で、符号は target 側に回転すべき方向を示す
	static float ShortestAngleDiff(float current, float target) {
		float diff = NormalizeAngle(target - current);
		return diff;
	}
}

void CameraManager::Initialize(Camera* camera)
{
	// メインカメラを登録する（nullptr 可）
	// 入力: camera - シーンで使用する Camera インスタンスへのポインタ
	// 副作用: mainCamera_ に保存し、以降の Update()/操作で参照する
    mainCamera_ = camera;
}

void CameraManager::Update()
{
	// 毎フレーム呼ぶ更新処理
	// - イージングが有効な場合はカメラ回転を easeTargetRotation_ へ少しずつ近づける
	// - 最終的に Camera::Update() を呼び、Camera 内の行列を更新させる
	//
	// 実装ノート:
	// - イージングは単純な線形補間 (LERP) で実装している（角度差は最短経路を考慮）
	// - 収束判定は角度差が angleEpsilon_ 未満になった場合とする

	if (mainCamera_ && isEasing_) {
		const Vector3 curRot = mainCamera_->GetRotate();
		// 現在のピッチ（X回転）とヨー（Y回転）
		float curPitch = curRot.x;
		float curYaw = curRot.y;

		// 目標との差を最短で取り、補間量を計算
		float diffPitch = ShortestAngleDiff(curPitch, easeTargetRotation_.x);
		float diffYaw = ShortestAngleDiff(curYaw, easeTargetRotation_.y);

		// 線形補間で新しい回転を求める
		float newPitch = curPitch + diffPitch * easeFactor_;
		float newYaw = curYaw + diffYaw * easeFactor_;

		// 目標に十分近づいたらイージング終了（ターゲット値へスナップ）
		if (std::fabs(diffPitch) < angleEpsilon_ && std::fabs(diffYaw) < angleEpsilon_) {
			isEasing_ = false;
			newPitch = easeTargetRotation_.x;
			newYaw = easeTargetRotation_.y;
		}

		// Z（ロール）は本実装では 0 に固定
		mainCamera_->SetRotate(Vector3(newPitch, newYaw, 0.0f));
	}

	// Camera 内で view/proj 行列を再計算
	if (mainCamera_) {
		mainCamera_->Update();
	}
}

void CameraManager::DrawImGui() {
#ifdef USE_IMGUI
    // デバッグ用 GUI: カメラ位置/回転/行列を表示・編集できる
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
    // 目的:
    // - 引数 target の位置に delta を加算し、同時にカメラの X 座標を delta.x 分だけ移動する簡易ヘルパ
    // 注意:
    // - この関数は target をコピーして受け取る設計になっている（呼び出し側のオブジェクトを直接更新しない）。
    // - より汎用的にしたい場合は参照渡し版を用意するか、カメラに対する移動処理を別に定義するとよい。
    
    target.translate_ += delta;
    
    if (mainCamera_) {
        mainCamera_->SetXPosition(mainCamera_->GetTranslate().x + delta.x);
    }
}

void CameraManager::LookAtTarget(const Vector3& targetPosition, bool eased, float easeFactor) {
    // 目的:
    // - カメラを targetPosition が中心に来るように向ける（Pitch/Yaw を計算して SetRotate）
    // - eased == false の場合は即時で向け、true の場合は内部のイージング状態を設定して Update で滑らかに回す
    //
    // 入力:
    // - targetPosition: 注視点のワールド座標
    // - eased: イージングを使うかどうか
    // - easeFactor: イージング時の補間係数（0..1 の範囲が期待値）
    //
    // 実装詳細:
    // - yaw = atan2f(dx, dz)
    // - pitch = -atan2f(dy, sqrt(dx*dx + dz*dz))
    //   （符号や atan2 の引数順は既存のワールド軸定義に合わせたもの）
    if (!mainCamera_) return;

    // カメラ位置と注視点の差分ベクトルを計算
    Vector3 camPos = mainCamera_->GetTranslate();
    Vector3 direction = targetPosition - camPos;

    // 回転角の計算（ラジアン）
    float yaw = atan2f(direction.x, direction.z);
    float pitch = -atan2f(direction.y, sqrtf(direction.x * direction.x + direction.z * direction.z));

    if (!eased) {
        // 即時適用：計算した角度をそのまま設定。ロールは 0 に固定。
        mainCamera_->SetRotate(Vector3(pitch, yaw, 0.0f));
        isEasing_ = false;
    }
    else {
        // イージング開始：目標回転を設定し、補間係数を保存してフラグを立てる
        easeTargetRotation_ = Vector3(pitch, yaw, 0.0f);
        easeFactor_ = easeFactor;
        // easeFactor の安全な範囲へクランプ
        if (easeFactor_ < 0.0f) easeFactor_ = 0.0f;
        if (easeFactor_ > 1.0f) easeFactor_ = 1.0f;
        isEasing_ = true;
    }
}


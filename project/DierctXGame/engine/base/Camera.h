#pragma once
#include <Transform.h>
#include <Matrix4x4.h>
#include <WinApp.h>
#include <Multiply.h>
// カメラ
class Camera
{
private:
	Transform transform;
	Matrix4x4 worldMatrix;
	Matrix4x4 viewMatrix;
	Matrix4x4 projectionMatrix;
	// 水平方向視野角
	float fovY = 0.45f;
	// アスペクト比
	float aspectRatio = float(WinApp::kClientWidth) / float(WinApp::kClientHeight);
	// ニアクリップ距離
	float nearClip = 0.1f;
	// ファークリップ距離
	float farClip = 100.0f;

	Matrix4x4 viewProjectionMatrix;

	
public:
	// デフォルトコンストラクタ
	Camera();
	// 更新
	void Update();

	// setter
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }
	void SetFovY(const float& fovY) { this->fovY = fovY; }
	void SetAspectRatio(const float& aspectRatio) { this->aspectRatio = aspectRatio; }
	void SetNearClip(const float& nearClip) { this->nearClip = nearClip; }
	void SetFarClip(const float& farClip) { this->farClip = farClip; }

	// getter
	const Matrix4x4& GetWorldMatrix()const { return worldMatrix; }
	const Matrix4x4& GetViewMatrix()const { return viewMatrix; }
	const Matrix4x4& GetProjectionMatrix()const { return projectionMatrix; }
	const Matrix4x4& GetViewProjectionMatrix()const { return viewProjectionMatrix; }
	const Vector3& GetRotate()const { return transform.rotate; }
	const Vector3& GetTranslate()const { return transform.translate; }
};


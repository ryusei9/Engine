#pragma once
#include <Transform.h>
#include <Matrix4x4.h>
#include <WinApp.h>
#include <Multiply.h>

// Camera用の定数
namespace CameraConstants {
	// デフォルトの視野角（ラジアン）
	constexpr float kDefaultFovY = 0.45f;
	
	// デフォルトのクリップ距離
	constexpr float kDefaultNearClip = 0.1f;
	constexpr float kDefaultFarClip = 100.0f;
	
	// デフォルトのスケール
	constexpr float kDefaultScaleX = 1.0f;
	constexpr float kDefaultScaleY = 1.0f;
	constexpr float kDefaultScaleZ = 1.0f;
	
	// デフォルトの回転
	constexpr float kDefaultRotateX = 0.0f;
	constexpr float kDefaultRotateY = 0.0f;
	constexpr float kDefaultRotateZ = 0.0f;
	
	// デフォルトの位置
	constexpr float kDefaultTranslateX = 0.0f;
	constexpr float kDefaultTranslateY = 0.0f;
	constexpr float kDefaultTranslateZ = 0.0f;
}

/// <summary>
/// カメラ
/// </summary>
class Camera
{
public:
	// デフォルトコンストラクタ
	Camera();
	
	// 更新
	void Update();

	// セッター
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }
	void SetXPosition(float x) { transform_.translate.x = x; }
	void SetFovY(float fovY) { fovY_ = fovY; }
	void SetAspectRatio(float aspectRatio) { aspectRatio_ = aspectRatio; }
	void SetNearClip(float nearClip) { nearClip_ = nearClip; }
	void SetFarClip(float farClip) { farClip_ = farClip; }

	// ゲッター
	const Matrix4x4& GetWorldMatrix() const { return worldMatrix_; }
	const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
	const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }
	const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix_; }
	const Vector3& GetRotate() const { return transform_.rotate; }
	const Vector3& GetTranslate() const { return transform_.translate; }
	float GetFovY() const { return fovY_; }
	float GetAspectRatio() const { return aspectRatio_; }

private:
	// 行列更新ヘルパー
	void UpdateWorldMatrix();
	void UpdateViewMatrix();
	void UpdateProjectionMatrix();
	void UpdateViewProjectionMatrix();
	
	// アスペクト比計算
	static float CalculateAspectRatio();

	// 変換情報
	Transform transform_;
	
	// カメラパラメータ
	float fovY_;
	float aspectRatio_;
	float nearClip_;
	float farClip_;

	// 行列
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 viewProjectionMatrix_;
};


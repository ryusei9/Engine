#pragma once
#include <Transform.h>
#include <Matrix4x4.h>
#include <WinApp.h>
#include <Multiply.h>

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

	/*------セッター------*/

	// 回転の設定	
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }

	// 平行移動の設定
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }

	// X座標の設定
	void SetXPosition(const float& x) { transform_.translate.x = x; }

	// 視野角の設定
	void SetFovY(const float& fovY) { fovY_ = fovY; }

	// アスペクト比の設定
	void SetAspectRatio(const float& aspectRatio) { aspectRatio_ = aspectRatio; }

	// ニアクリップ距離の設定
	void SetNearClip(const float& nearClip) { nearClip_ = nearClip; }

	// ファークリップ距離の設定
	void SetFarClip(const float& farClip) { farClip_ = farClip; }

	/*------ゲッター------*/

	// ワールド行列の取得
	const Matrix4x4& GetWorldMatrix() const { return worldMatrix_; }

	// ビュー行列の取得
	const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }

	// 射影行列の取得
	const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }

	// ビュー射影行列の取得
	const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix_; }

	// 回転の取得
	const Vector3& GetRotate() const { return transform_.rotate; }

	// 平行移動の取得
	const Vector3& GetTranslate() const { return transform_.translate; }

	// 視野角の取得
	const float& GetFovY() const { return fovY_; }

	// アスペクト比の取得
	const float& GetAspectRatio() const { return aspectRatio_; }

private:
	// 変換情報
	Transform transform_;
	
	// 水平方向視野角
	float fovY_ = 0.45f;
	// アスペクト比
	float aspectRatio_ = float(WinApp::kClientWidth) / float(WinApp::kClientHeight);
	// ニアクリップ距離
	float nearClip_ = 0.1f;
	// ファークリップ距離
	float farClip_ = 100.0f;

	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 viewProjectionMatrix_;
};


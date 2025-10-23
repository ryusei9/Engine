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
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }

	// 平行移動の設定
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }

	// 視野角の設定
	void SetFovY(const float& fovY) { this->fovY = fovY; }

	// アスペクト比の設定
	void SetAspectRatio(const float& aspectRatio) { this->aspectRatio = aspectRatio; }

	// ニアクリップ距離の設定
	void SetNearClip(const float& nearClip) { this->nearClip = nearClip; }

	// ファークリップ距離の設定
	void SetFarClip(const float& farClip) { this->farClip = farClip; }

	/*------ゲッター------*/

	// ワールド行列の取得
	const Matrix4x4& GetWorldMatrix()const { return worldMatrix; }

	// ビュー行列の取得
	const Matrix4x4& GetViewMatrix()const { return viewMatrix; }

	// 射影行列の取得
	const Matrix4x4& GetProjectionMatrix()const { return projectionMatrix; }

	// ビュー射影行列の取得
	const Matrix4x4& GetViewProjectionMatrix()const { return viewProjectionMatrix; }

	// 回転の取得
	const Vector3& GetRotate()const { return transform.rotate; }

	// 平行移動の取得
	const Vector3& GetTranslate()const { return transform.translate; }

private:

	// 変換情報
	Transform transform;
	
	// 水平方向視野角
	float fovY = 0.45f;
	// アスペクト比
	float aspectRatio = float(WinApp::kClientWidth) / float(WinApp::kClientHeight);
	// ニアクリップ距離
	float nearClip = 0.1f;
	// ファークリップ距離
	float farClip = 100.0f;

	
	Matrix4x4 worldMatrix;
	Matrix4x4 viewMatrix;
	Matrix4x4 projectionMatrix;
	Matrix4x4 viewProjectionMatrix;

};


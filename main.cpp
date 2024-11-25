#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <dxgidebug.h>
#include <dxcapi.h>
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#include "externals/DirectXTex/DirectXTex.h"
#include <numbers>
#include <fstream>
#include <sstream>
#include <wrl.h>
#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "D3DresourceLeakChecker.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix3x3.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include "VertexData.h"
#include "MaterialData.h"
#include "ModelData.h"
#include "TextureManager.h"
#include "Object3dCommon.h"
#include "Object3d.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

// ウィンドウプロシーシャ
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg,
	WPARAM wparam, LPARAM lparam) {
	// ImGuiにメッセージを渡す
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
	// メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		// ウィンドウが破棄された
	case WM_DESTROY:
		// OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	// 標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

struct Sphere {
	Vector3 center;
	float radius;
};





struct DirectionalLight {
	Vector4 color; // ライトの色
	Vector3 direction; // ライトの向き
	float intensity; // 輝度
};

struct D3DResourceLeakChecker {
	~D3DResourceLeakChecker() {
		// リソースリリースチェック
		Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
			debug->Release();
		}
	}
};

class ResourceObject {
public:
	ResourceObject(Microsoft::WRL::ComPtr<ID3D12Resource> resource)
		: resource_(resource)
	{}
	~ResourceObject() {
		if (resource_) {
			resource_->Release();
		}
	}
	Microsoft::WRL::ComPtr<ID3D12Resource> Get() { return resource_; }
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
};


Matrix4x4 MakeRotateXMatrix(float radian);
Matrix4x4 MakeRotateYMatrix(float radian);
Matrix4x4 MakeRotateZMatrix(float radian);
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3 translate);


// アフィン変換
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3 translate) {
	Matrix4x4 resultAffineMatrix = {};
	Matrix4x4 resultRotateXYZMatrix = Multiply(MakeRotateXMatrix(rotate.x), Multiply(MakeRotateYMatrix(rotate.y), MakeRotateZMatrix(rotate.z)));
	resultAffineMatrix.m[0][0] = scale.x * resultRotateXYZMatrix.m[0][0];
	resultAffineMatrix.m[0][1] = scale.x * resultRotateXYZMatrix.m[0][1];
	resultAffineMatrix.m[0][2] = scale.x * resultRotateXYZMatrix.m[0][2];
	resultAffineMatrix.m[1][0] = scale.y * resultRotateXYZMatrix.m[1][0];
	resultAffineMatrix.m[1][1] = scale.y * resultRotateXYZMatrix.m[1][1];
	resultAffineMatrix.m[1][2] = scale.y * resultRotateXYZMatrix.m[1][2];
	resultAffineMatrix.m[2][0] = scale.z * resultRotateXYZMatrix.m[2][0];
	resultAffineMatrix.m[2][1] = scale.z * resultRotateXYZMatrix.m[2][1];
	resultAffineMatrix.m[2][2] = scale.z * resultRotateXYZMatrix.m[2][2];
	resultAffineMatrix.m[3][0] = translate.x;
	resultAffineMatrix.m[3][1] = translate.y;
	resultAffineMatrix.m[3][2] = translate.z;
	resultAffineMatrix.m[3][3] = 1;
	return resultAffineMatrix;
}

// x軸回転行列
Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 resultRotateXMatrix = {};
	resultRotateXMatrix.m[0][0] = 1;
	resultRotateXMatrix.m[1][1] = std::cos(radian);
	resultRotateXMatrix.m[1][2] = std::sin(radian);
	resultRotateXMatrix.m[2][1] = -std::sin(radian);
	resultRotateXMatrix.m[2][2] = std::cos(radian);
	resultRotateXMatrix.m[3][3] = 1;
	return resultRotateXMatrix;
}

// y軸回転行列
Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 resultRotateYMatrix = {};
	resultRotateYMatrix.m[0][0] = std::cos(radian);
	resultRotateYMatrix.m[0][2] = -std::sin(radian);
	resultRotateYMatrix.m[1][1] = 1;
	resultRotateYMatrix.m[2][0] = std::sin(radian);
	resultRotateYMatrix.m[2][2] = std::cos(radian);
	resultRotateYMatrix.m[3][3] = 1;
	return resultRotateYMatrix;
}

// z軸回転行列
Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 resultRotateZMatrix = {};
	resultRotateZMatrix.m[0][0] = std::cos(radian);
	resultRotateZMatrix.m[0][1] = std::sin(radian);
	resultRotateZMatrix.m[1][0] = -std::sin(radian);
	resultRotateZMatrix.m[1][1] = std::cos(radian);
	resultRotateZMatrix.m[2][2] = 1;
	resultRotateZMatrix.m[3][3] = 1;
	return resultRotateZMatrix;
}

// 行列の積
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 resultMultiply = {};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			resultMultiply.m[i][j] = m1.m[i][0] * m2.m[0][j] + m1.m[i][1] * m2.m[1][j] + m1.m[i][2] * m2.m[2][j] + m1.m[i][3] * m2.m[3][j];
		}
	}
	return resultMultiply;
}

// 透視投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 resultPerspectiveFovMatrix = {};
	resultPerspectiveFovMatrix.m[0][0] = (1 / aspectRatio) * (1 / std::tan(fovY / 2));
	resultPerspectiveFovMatrix.m[1][1] = 1 / std::tan(fovY / 2);
	resultPerspectiveFovMatrix.m[2][2] = farClip / (farClip - nearClip);
	resultPerspectiveFovMatrix.m[2][3] = 1;
	resultPerspectiveFovMatrix.m[3][2] = -nearClip * farClip / (farClip - nearClip);
	return resultPerspectiveFovMatrix;
}

// 逆行列
Matrix4x4 Inverse(const Matrix4x4& m) {
	Matrix4x4 resultInverse = {};
	float A = m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1] + m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2]
		- m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1] - m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2]
		- m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1] - m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2]
		+ m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1] + m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2]
		+ m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1] + m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2]
		- m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1] - m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2]
		- m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0] - m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0] - m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0]
		+ m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0] + m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0] + m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0];
	resultInverse.m[0][0] = (
		m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[1][2] * m.m[2][3] * m.m[3][1] + m.m[1][3] * m.m[2][1] * m.m[3][2]
		- m.m[1][3] * m.m[2][2] * m.m[3][1] - m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[1][1] * m.m[2][3] * m.m[3][2]) / A;
	resultInverse.m[0][1] = (
		-m.m[0][1] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[2][3] * m.m[3][1] - m.m[0][3] * m.m[2][1] * m.m[3][2]
		+ m.m[0][3] * m.m[2][2] * m.m[3][1] + m.m[0][2] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[2][3] * m.m[3][2]) / A;
	resultInverse.m[0][2] = (
		m.m[0][1] * m.m[1][2] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[3][1] + m.m[0][3] * m.m[1][1] * m.m[3][2]
		- m.m[0][3] * m.m[1][2] * m.m[3][1] - m.m[0][2] * m.m[1][1] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[3][2]) / A;
	resultInverse.m[0][3] = (
		-m.m[0][1] * m.m[1][2] * m.m[2][3] - m.m[0][2] * m.m[1][3] * m.m[2][1] - m.m[0][3] * m.m[1][1] * m.m[2][2]
		+ m.m[0][3] * m.m[1][2] * m.m[2][1] + m.m[0][2] * m.m[1][1] * m.m[2][3] + m.m[0][1] * m.m[1][3] * m.m[2][2]) / A;
	resultInverse.m[1][0] = (
		-m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[1][2] * m.m[2][3] * m.m[3][0] - m.m[1][3] * m.m[2][0] * m.m[3][2]
		+ m.m[1][3] * m.m[2][2] * m.m[3][0] + m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[1][0] * m.m[2][3] * m.m[3][2]) / A;
	resultInverse.m[1][1] = (
		m.m[0][0] * m.m[2][2] * m.m[3][3] + m.m[0][2] * m.m[2][3] * m.m[3][0] + m.m[0][3] * m.m[2][0] * m.m[3][2]
		- m.m[0][3] * m.m[2][2] * m.m[3][0] - m.m[0][2] * m.m[2][0] * m.m[3][3] - m.m[0][0] * m.m[2][3] * m.m[3][2]) / A;
	resultInverse.m[1][2] = (
		-m.m[0][0] * m.m[1][2] * m.m[3][3] - m.m[0][2] * m.m[1][3] * m.m[3][0] - m.m[0][3] * m.m[1][0] * m.m[3][2]
		+ m.m[0][3] * m.m[1][2] * m.m[3][0] + m.m[0][2] * m.m[1][0] * m.m[3][3] + m.m[0][0] * m.m[1][3] * m.m[3][2]) / A;
	resultInverse.m[1][3] = (
		m.m[0][0] * m.m[1][2] * m.m[2][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] + m.m[0][3] * m.m[1][0] * m.m[2][2]
		- m.m[0][3] * m.m[1][2] * m.m[2][0] - m.m[0][2] * m.m[1][0] * m.m[2][3] - m.m[0][0] * m.m[1][3] * m.m[2][2]) / A;
	resultInverse.m[2][0] = (
		m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[1][1] * m.m[2][3] * m.m[3][0] + m.m[1][3] * m.m[2][0] * m.m[3][1]
		- m.m[1][3] * m.m[2][1] * m.m[3][0] - m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[1][0] * m.m[2][3] * m.m[3][1]) / A;
	resultInverse.m[2][1] = (
		-m.m[0][0] * m.m[2][1] * m.m[3][3] - m.m[0][1] * m.m[2][3] * m.m[3][0] - m.m[0][3] * m.m[2][0] * m.m[3][1]
		+ m.m[0][3] * m.m[2][1] * m.m[3][0] + m.m[0][1] * m.m[2][0] * m.m[3][3] + m.m[0][0] * m.m[2][3] * m.m[3][1]) / A;
	resultInverse.m[2][2] = (
		m.m[0][0] * m.m[1][1] * m.m[3][3] + m.m[0][1] * m.m[1][3] * m.m[3][0] + m.m[0][3] * m.m[1][0] * m.m[3][1]
		- m.m[0][3] * m.m[1][1] * m.m[3][0] - m.m[0][1] * m.m[1][0] * m.m[3][3] - m.m[0][0] * m.m[1][3] * m.m[3][1]) / A;
	resultInverse.m[2][3] = (
		-m.m[0][0] * m.m[1][1] * m.m[2][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] - m.m[0][3] * m.m[1][0] * m.m[2][1]
		+ m.m[0][3] * m.m[1][1] * m.m[2][0] + m.m[0][1] * m.m[1][0] * m.m[2][3] + m.m[0][0] * m.m[1][3] * m.m[2][1]) / A;
	resultInverse.m[3][0] = (
		-m.m[1][0] * m.m[2][1] * m.m[3][2] - m.m[1][1] * m.m[2][2] * m.m[3][0] - m.m[1][2] * m.m[2][0] * m.m[3][1]
		+ m.m[1][2] * m.m[2][1] * m.m[3][0] + m.m[1][1] * m.m[2][0] * m.m[3][2] + m.m[1][0] * m.m[2][2] * m.m[3][1]) / A;
	resultInverse.m[3][1] = (
		m.m[0][0] * m.m[2][1] * m.m[3][2] + m.m[0][1] * m.m[2][2] * m.m[3][0] + m.m[0][2] * m.m[2][0] * m.m[3][1]
		- m.m[0][2] * m.m[2][1] * m.m[3][0] - m.m[0][1] * m.m[2][0] * m.m[3][2] - m.m[0][0] * m.m[2][2] * m.m[3][1]) / A;
	resultInverse.m[3][2] = (
		-m.m[0][0] * m.m[1][1] * m.m[3][2] - m.m[0][1] * m.m[1][2] * m.m[3][0] - m.m[0][2] * m.m[1][0] * m.m[3][1]
		+ m.m[0][2] * m.m[1][1] * m.m[3][0] + m.m[0][1] * m.m[1][0] * m.m[3][2] + m.m[0][0] * m.m[1][2] * m.m[3][1]) / A;
	resultInverse.m[3][3] = (
		m.m[0][0] * m.m[1][1] * m.m[2][2] + m.m[0][1] * m.m[1][2] * m.m[2][0] + m.m[0][2] * m.m[1][0] * m.m[2][1]
		- m.m[0][2] * m.m[1][1] * m.m[2][0] - m.m[0][1] * m.m[1][0] * m.m[2][2] - m.m[0][0] * m.m[1][2] * m.m[2][1]) / A;
	return resultInverse;
}

// 正射影行列
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 resultOrthographicMatrix = {};
	resultOrthographicMatrix.m[0][0] = 2 / (right - left);
	resultOrthographicMatrix.m[1][1] = 2 / (top - bottom);
	resultOrthographicMatrix.m[2][2] = 1 / (farClip - nearClip);
	resultOrthographicMatrix.m[3][0] = (left + right) / (left - right);
	resultOrthographicMatrix.m[3][1] = (top + bottom) / (bottom - top);
	resultOrthographicMatrix.m[3][2] = nearClip / (nearClip - farClip);
	resultOrthographicMatrix.m[3][3] = 1;
	return resultOrthographicMatrix;
}

Vector3 TransformMatrix(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 resultTransform = {};
	resultTransform.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	resultTransform.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	resultTransform.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];

	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];
	assert(w != 0.0f);
	resultTransform.x /= w;
	resultTransform.y /= w;
	resultTransform.z /= w;

	return resultTransform;
}

// 正規化
Vector3 Normalize(const Vector3& v) {
	Vector3 NormalizeResult = {};
	float LengthResult = {};
	LengthResult = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	NormalizeResult.x = v.x / LengthResult;
	NormalizeResult.y = v.y / LengthResult;
	NormalizeResult.z = v.z / LengthResult;
	return NormalizeResult;
}

Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 resultScale = {};
	resultScale.m[0][0] = scale.x;
	resultScale.m[1][1] = scale.y;
	resultScale.m[2][2] = scale.z;
	resultScale.m[3][3] = 1;
	return resultScale;
}

Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 resultTranslate = {};
	resultTranslate.m[3][0] = translate.x;
	resultTranslate.m[3][1] = translate.y;
	resultTranslate.m[3][2] = translate.z;
	resultTranslate.m[0][0] = 1;
	resultTranslate.m[1][1] = 1;
	resultTranslate.m[2][2] = 1;
	resultTranslate.m[3][3] = 1;
	return resultTranslate;
}

// std::stringを受け取る関数
void Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}

std::wstring ConvertString(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}

std::string ConvertString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}

// BufferResourceの作成
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes) {
	// DXGIファクトリーの生成
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	// 関数が成功したかどうか
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};

	// UploadHeapを使う
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	// 頂点リソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};

	// バッファリソース。テクスチャの場合はまた別の設定をする
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	// リソースのサイズ
	resourceDesc.Width = sizeInBytes;

	// バッファの場合はこれらは1にする決まり
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;

	// バッファの場合はこれにする決まり
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;



	// 実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

// CompileShader関数
IDxcBlob* CompileShader(
	// compilerするshaderファイルへのパス
	const std::wstring& filePath,
	// compilerに使用するprofile
	const wchar_t* profile,
	// 初期化で生成したものを3つ
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler
) {
	// これからシェーダーをコンパイルする旨をログに出す
	Log(ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));
	// hlslファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	// 読めなかったら止める
	assert(SUCCEEDED(hr));
	// 読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	// UTF8の文字コードであることを通知
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;
	// compileする
	LPCWSTR arguments[] = {
		// コンパイル対象のhlslファイル名
		filePath.c_str(),
		// エントリーポイントの指定(基本main以外にはしない)
		L"-E",L"main",
		// shaderProfileの設定
		L"-T",profile,
		// デバッグ用の情報を埋め込む
		L"-Zi",L"-Qembed_debug",
		// 最適化を外しておく
		L"-Od",
		// メモリレイアウトは行優先
		L"-Zpr",
	};
	// 実際にshaderをコンパイルする
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		// 読み込んだファイル
		&shaderSourceBuffer,
		// コンパイルオプション
		arguments,
		// コンパイルオプションの数
		_countof(arguments),
		// includeが含まれた諸々
		includeHandler,
		// コンパイル結果
		IID_PPV_ARGS(&shaderResult)
	);
	// コンパイルエラーではなくdxcが起動できないなど致命的な状況
	assert(SUCCEEDED(hr));
	// 警告・エラーが出ていないか確認する
	// 警告・エラーが出てたらログに出して止める
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		// 警告・エラーは駄目です
		assert(false);
	}
	// compile結果を受け取って渡す
	// コンパイル結果から実行用のバイナリ部分を取得
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	// 成功したログを出す
	Log(ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));
	// もう使わないリソースを解放
	shaderSource->Release();
	shaderResult->Release();
	// 実行用のバイナリを返却
	return shaderBlob;
}

// DirectXTexを使ってTextureを読むためのLoadTexture関数
DirectX::ScratchImage LoadTexture(const std::string& filePath) {
	// テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);

	// ミップマップ付きのデータを渡す
	return mipImages;
}

// DirectX12のTextureResourceを作る
Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metadata) {
	// metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	// Textureの幅
	resourceDesc.Width = UINT(metadata.width);
	// Textureの高さ
	resourceDesc.Height = UINT(metadata.height);
	// mipmapの数
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);
	// 奥行き or 配列Textureの配列数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);
	// TextureのFormat
	resourceDesc.Format = metadata.format;
	// サンプリングカウント。1固定
	resourceDesc.SampleDesc.Count = 1;
	// textureの次元数。普段使っているのは2次元
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	// 細かい設定を行う
	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
	// WriteBackポリシーでCPUアクセス可能
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	// プロセッサの近くに配置
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

	// Resourceを生成する
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		// Heapの設定
		&heapProperties,
		// Heapの特殊な設定。特になし
		D3D12_HEAP_FLAG_NONE,
		// Resourceの設定
		&resourceDesc,
		// 初回のResourceState。Textureは基本読むだけ
		D3D12_RESOURCE_STATE_GENERIC_READ,
		// Clear最適値。使わないのでnullptr
		nullptr,
		// 作成するResourceポインタへのポインタ
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

// TextureResourceにデータを転送する
void UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages) {
	// meta情報を取得
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	// MipMapについて
	for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {
		// MipMapLevelｗｐ指定して各Imageを取得
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
		// Textureに転送
		HRESULT hr = texture->WriteToSubresource(
			UINT(mipLevel),
			// 全領域へコピー
			nullptr,
			// 元データアドレス
			img->pixels,
			// 1ラインサイズ
			UINT(img->rowPitch),
			// 1枚サイズ
			UINT(img->slicePitch)
		);
		assert(SUCCEEDED(hr));
	}
}

// DescriptorHeapを取得する
// CPU
D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}
// GPU
D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	// 変数の宣言
	// 構築するMaterialData
	MaterialData materialData;
	// ファイルから読んだ一行を格納するもの
	std::string line;
	// ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);
	// 開けなかったら止める
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;

			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}
	return materialData;
}

// objファイルを読む関数
ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	// 変数の宣言
	// 構築するModelData
	ModelData modelData;
	// 位置
	std::vector<Vector4> positions;
	// 法線
	std::vector<Vector3> normals;
	// テクスチャ座標
	std::vector<Vector2> texcoords;
	// ファイルから読んだ一行を格納するもの
	std::string line;

	// ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);
	// 開けなかったら止める
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		// 先頭の識別子を読む
		s >> identifier;

		// identifierに応じた処理
		// 頂点位置
		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.x *= -1.0f;
			position.w = 1.0f;
			positions.push_back(position);
			// 頂点テクスチャ座標
		} else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
			// 頂点法線
		} else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f;
			normals.push_back(normal);
			// 面
		} else if (identifier == "f") {
			VertexData triangle[3];
			// 面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					// /区切りでインデックスを読んでいく
					std::getline(v, index, '/');
					elementIndices[element] = std::stoi(index);
				}
				// 要素へのIndexから、実際の要素の値を取得して、頂点を構築していく
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				/*VertexData vertex = { position,texcoord,normal };
				modelData.vertices.push_back(vertex);*/
				triangle[faceVertex] = { position,texcoord,normal };
			}
			// 頂点を逆順で登録することで、回り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		} else if (identifier == "mtllib") {
			// materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;

			// 基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	return modelData;
}


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	// COMの初期化
	CoInitializeEx(0, COINIT_MULTITHREADED);

	// COMの初期化
	//HRESULT hr;

	// ポインタ
	WinApp* winApp = nullptr;

	// WindowsAPIの初期化
	winApp = new WinApp();
	winApp->Initialize();


	// 出力ウィンドウへの文字出力
	OutputDebugStringA("Hello,DirectX!\n");

	// 文字列を格納する
	std::string str0{ "STRING!!!" };

	// 整数を文字列にする
	std::string str1{ std::to_string(10) };

	// 変数から型を推論してくれる
	Log(std::format("str0:{},str1:{}\n", str0, str1));

	

	// ポインタ
	DirectXCommon* dxCommon = nullptr;

	// DirectXの初期化
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	// テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(dxCommon);

	// テクスチャを事前にロード
	TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");
	TextureManager::GetInstance()->LoadTexture("resources/monsterBall.png");
	////////////////////////
	// input
	////////////////////////
	// ポインタ
	Input* input = nullptr;

	// 入力の初期化
	input = new Input();
	input->Initialize(winApp);


	SpriteCommon* spriteCommon = nullptr;
	// スプライト共通部の初期化
	spriteCommon = new SpriteCommon;
	spriteCommon->Initialize(dxCommon);

	Object3dCommon* object3dCommon = nullptr;
	// 3Dオブジェクト共通部の初期化
	object3dCommon = new Object3dCommon;
	object3dCommon->Initialize(dxCommon);



	//////////////////////////
	// VertexResourceを生成する
	//////////////////////////
	// モデル読み込み
	ModelData modelData = LoadObjFile("resources", "axis.obj");
	// リソースの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = CreateBufferResource(dxCommon->GetDevice(), sizeof(VertexData) * modelData.vertices.size());

	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();

	// 使用するリソースのサイズは頂点のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());

	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;

	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// 頂点データをリソースにコピー
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	//CreateBufferResource(device, sizeof(float) * 3);

	//// WVP用のリソースを作る。
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = CreateBufferResource(dxCommon->GetDevice(), sizeof(TransformationMatrix));

	//// DepthStencilTextureをウィンドウのサイズで作成
	//Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = CreateDepthStencilTextureResource(device, WinApp::kClientWidth, WinApp::kClientHeight);

	// データを書き込む
	TransformationMatrix* wvpData = nullptr;

	// 書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));

	// 単位行列を書き込んでおく
	wvpData->WVP = MakeIdentity4x4::MakeIdentity4x4();

	/// Lightingの設定
	Material* materialDataSprite = nullptr;

	// Sprite用のマテリアルリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = CreateBufferResource(dxCommon->GetDevice(), sizeof(Material));

	// ...Mapしてデータを書き込む。色は白を設定しておくといい
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));

	materialDataSprite->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	// SpriteはLightingしないのでfalseを設定する
	materialDataSprite->enableLighting = false;

	// 単位行列で初期化
	materialDataSprite->uvTransform = MakeIdentity4x4::MakeIdentity4x4();

	// 平行光源用のリソースを作成
	DirectionalLight* directionalLightData = nullptr;

	// マテリアルリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = CreateBufferResource(dxCommon->GetDevice(), sizeof(DirectionalLight));

	// 書き込むためのアドレスを取得
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));

	// 真上から白いライトで照らす
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightData->intensity = 1.0f;

	// 正規化
	Normalize(directionalLightData->direction);

	


	
	

#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(dxCommon->GetDevice()->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		// ヤバいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		// エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		// 警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		// 抑制するメッセージ
		D3D12_MESSAGE_ID denyIds[] = {
			// Windows11でのDXGIデバッグレイヤーの相互作用バグによるエラーメッセージ
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		// 抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};

		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		// 指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);
		// 解放
		infoQueue->Release();
	}
#endif

	

	//////////////////////////
	// Textureを読む
	//////////////////////////
	// Textureを読んで転送する
	//DirectX::ScratchImage mipImages[2];
	//mipImages[0] = LoadTexture("resources/uvChecker.png");
	//const DirectX::TexMetadata& metadata = mipImages[0].GetMetadata();
	//Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = CreateTextureResource(dxCommon->GetDevice(), metadata);
	//UploadTextureData(textureResource.Get(), mipImages[0]);

	//// 2枚目のTextureを読んで転送する
	//mipImages[1] = LoadTexture(modelData.material.textureFilePath);
	//const DirectX::TexMetadata& metadata2 = mipImages[1].GetMetadata();
	//Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = CreateTextureResource(dxCommon->GetDevice(), metadata2);
	//UploadTextureData(textureResource2.Get(), mipImages[1]);

	////////////////////////////////////
	//// ShaderResourceViewを作る
	////////////////////////////////////
	//// metadataを基にSRVの設定
	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	//srvDesc.Format = metadata.format;
	//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//// 2Dテクスチャ
	//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	//// SRVを作成するDescriptorHeapの場所を決める
	//D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = GetCPUDescriptorHandle(dxCommon->GetSRVDescriptorHeap().Get(), dxCommon->GetDescriptorSizeSRV(), 1);
	//D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = GetGPUDescriptorHandle(dxCommon->GetSRVDescriptorHeap().Get(), dxCommon->GetDescriptorSizeSRV(), 1);
	//// 先頭はImGuiが使っているのでその次を使う
	///*textureSrvHandleCPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//textureSrvHandleGPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);*/

	//// SRVの生成
	//dxCommon->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);

	//// metadata2を基にSRVの設定
	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	//srvDesc2.Format = metadata2.format;
	//srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//// 2Dテクスチャ
	//srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	//// SRVを作成するDescriptorHeapの場所を決める
	//D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = GetCPUDescriptorHandle(dxCommon->GetSRVDescriptorHeap().Get(), dxCommon->GetDescriptorSizeSRV(), 2);
	//D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = GetGPUDescriptorHandle(dxCommon->GetSRVDescriptorHeap().Get(), dxCommon->GetDescriptorSizeSRV(), 2);

	//// SRVの生成
	//dxCommon->GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);

	// Transform変数を作る
	Transform transform{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	Transform cameraTransform{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,-10.0f}
	};

	
	Transform uvTransformSprite{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	bool useMonsterBall = true;

	/*Sprite* sprite = new Sprite();
	sprite->Initialize(spriteCommon, dxCommon);*/

	std::vector<Sprite*> sprites;
	for (uint32_t i = 0; i < 5; ++i) {
		if (i < 4) {
			Sprite* sprite = new Sprite();
			sprite->Initialize(spriteCommon, dxCommon, "resources/uvChecker.png");
			sprites.push_back(sprite);
		} else {
			Sprite* sprite = new Sprite();
			sprite->Initialize(spriteCommon, dxCommon, "resources/monsterBall.png");
			sprites.push_back(sprite);
		}
	}

	Object3d* object3d = new Object3d();
	object3d->Initialize();
	MSG msg{};
	// ウィンドウの×ボタンが押されるまでループ
	while (true) {
		// Windowsのメッセージ処理
		if (winApp->ProcessMessage()) {
			// ゲームループを抜ける
			break;
		}
		// ImGuiにここからフレームが始まる旨を伝える
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// 開発用の処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
		//ImGui::ShowDemoWindow();
		ImGui::DragFloat3("cameraPosition", &cameraTransform.translate.x, 0.01f);
		ImGui::SliderAngle("modelRotate.x", &transform.rotate.x);
		ImGui::SliderAngle("modelRotate.y", &transform.rotate.y);
		ImGui::SliderAngle("modelRotate.z", &transform.rotate.z);
		ImGui::ColorEdit3("modelColor", &materialDataSprite->color.x);
		/*ImGui::Checkbox("useMonsterBall", &useMonsterBall);
		ImGui::ColorEdit3("lightColor", &directionalLightData->color.x);
		ImGui::DragFloat3("lightDirection", &directionalLightData->direction.x, 0.01f);
		ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);*/

		
		// ゲームの処理
		// 入力の更新
		input->Update();
		// 操作
		if (input->TriggerKey(DIK_RIGHTARROW)) {
			cameraTransform.translate.x -= 1.0f;
		}
		if (input->PushKey(DIK_LEFTARROW)) {
			cameraTransform.translate.x += 0.01f;
		}
		//transform.rotate.y += 0.01f;
		for (uint32_t i = 0; i < sprites.size();++i) {

			sprites[i]->Update();

			// 現在の座標を変数で受ける
			Vector2 position = sprites[i]->GetPosition();

			// 座標を変更する
			ImGui::DragFloat2("sprite.translate", &position.x);
			// 変更を反映する
			sprites[i]->SetPosition({200.0f * i});

			// 角度を変更させるテスト
			float rotation = sprites[i]->GetRotation();

			ImGui::DragFloat("sprites.rotation", &rotation, 0.01f);

			sprites[i]->SetRotation(rotation);

			// 色を変化させるテスト
			Vector4 color = sprites[i]->GetColor();

			ImGui::ColorEdit3("sprites.color", &color.x);

			sprites[i]->SetColor(color);

			// サイズを変化させるテスト
			Vector2 size = sprites[i]->GetSize();
			ImGui::DragFloat2("sprites.scale", &size.x);

			sprites[i]->SetSize({100.0f,100.0f});

			// 反転X
			bool isFlipX = sprites[i]->GetIsFlipX();
			ImGui::Checkbox("sprites.isFlipX", &isFlipX);

			sprites[i]->SetIsFlipX(isFlipX);

			bool isFlipY = sprites[i]->GetIsFlipY();
			ImGui::Checkbox("sprites.isFlipY", &isFlipY);

			sprites[i]->SetIsFlipY(isFlipY);
		}
		Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
		materialDataSprite->uvTransform = uvTransformMatrix;

		ImGui::Render();
		/////////////////////
		//// コマンドをキック
		/////////////////////

		// 描画前処理
		dxCommon->PreDraw();

		// 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックコマンドを積む
		object3dCommon->DrawSettings();
		// 全てのobject3d個々の描画
		
		// Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
		spriteCommon->DrawSettings();
		// RootSignatureを設定
		//dxCommon->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
		//// PSOを設定
		//dxCommon->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
		// VBVを設定
		//dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);

		// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考える
		//dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// マテリアルCBufferの場所を設定

		//// 第一引数の0はRootParameter配列の0番目
		//dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

		////// wvp用のCBufferの場所を設定
		//dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());

		//// Lightingを行うSprite
		//dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, transformationMatrixResourceSprite->GetGPUVirtualAddress());

		//// 平行光源
		//dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

		//// SRVのDescriptorTableの先頭を設定。
		//dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);

		//// 描画先のRTVとDSVを設定する
		//D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxCommon->GetDSVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();

		//// 指定した深度で画面全体をクリアする
		//dxCommon->GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		//// 描画 (DrawCall)。3頂点で1つのインスタンス。
		//dxCommon->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);

		/////////////////////
		/// sprite
		/////////////////////

		// Spriteの描画。変更が必要なものだけ変更する
		// VBVを設定
		//dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
		//dxCommon->GetCommandList()->IASetIndexBuffer(&indexBufferViewSprite);

		//dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());

		//// TransformationMatrixCBufferの場所を設定
		//dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
		//// Spriteを常にuvCheckerにする
		//dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
		//// 描画
		//dxCommon->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0,0);
		for (auto& sprite : sprites) {
			sprite->Draw();
		}
		dxCommon->PostDraw();
		

	}
	
	////////////////////
	// 解放処理
	////////////////////
	// 入力解放
	delete object3d;
	delete input;
	delete spriteCommon;
	for (auto& sprite : sprites) {
		delete sprite;
	}
	delete object3dCommon;
	// WindowsAPIの終了処理
	winApp->Finalize();
	delete winApp;

	CloseHandle(dxCommon->GetFenceEvent());
	delete dxCommon;

	// テクスチャマネージャの終了
	TextureManager::GetInstance()->Finalize();

	// ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	return 0;
}
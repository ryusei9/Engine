#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <dxgidebug.h>
#include <dxcapi.h>

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
#include "ModelCommon.h"
#include "Model.h"
#include "ModelManager.h"
#include "Camera.h"
#include <SrvManager.h>
#include "ImGuiManager.h"
#include "imgui.h"
#include <xaudio2.h>
#include "Audio.h"

#pragma comment(lib,"xaudio2.lib")

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")


// ゲーム全体
class MyGame
{
public:

	// 初期化
	void Initialize();

	// 終了
	void Finelize();

	// 毎フレーム更新
	void Update();

	// 描画
	void Draw();

	// 終了リクエスト
	bool IsEndRequest() const { return endRequest_; }

private:
	// メンバ変数
	// ポインタ
	WinApp* winApp = nullptr;

	DirectXCommon* dxCommon = nullptr;

	SrvManager* srvManager = nullptr;

	Input* input = nullptr;

	SpriteCommon* spriteCommon = nullptr;

	Object3dCommon* object3dCommon = nullptr;

	Camera* camera = new Camera();

	ImGuiManager* imGuiManager = new ImGuiManager();

	Sprite* sprite = new Sprite();



	Transform cameraTransform{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,-10.0f}
	};

	bool useMonsterBall = true;

	Vector2 spritePosition = { 100.0f,100.0f };

	SoundData soundData1;

	// ゲーム終了フラグ
	bool endRequest_ = false;
};


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
#include "SRFramework.h"

#pragma comment(lib,"xaudio2.lib")

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")
// ゲーム全体
class SRFramework
{
public:
	virtual ~SRFramework() = default;

	// 初期化
	virtual void Initialize();

	// 終了
	virtual void Finelize();

	// 毎フレーム更新
	virtual void Update();

	// 描画
	virtual void Draw() = 0;

	// 終了リクエスト
	virtual bool IsEndRequest() const { return endRequest_; }

	// 実行
	void Run();
private:
	// メンバ変数
	// ポインタ
	WinApp* winApp = nullptr;

	DirectXCommon* dxCommon = nullptr;

	SrvManager* srvManager = nullptr;

	//Input* input = nullptr;

	SpriteCommon* spriteCommon = nullptr;

	Object3dCommon* object3dCommon = nullptr;

	Camera* camera = new Camera();

	ImGuiManager* imGuiManager = new ImGuiManager();

	Sprite* sprite = new Sprite();

	bool endRequest_ = false;

};


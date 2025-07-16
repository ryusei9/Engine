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
#include <SceneManager.h>

#pragma comment(lib,"xaudio2.lib")

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")
using namespace std;
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

	// 描画前処理
	virtual void PreDraw();

	// 描画後処理
	virtual void PostDraw();

	// 終了リクエスト
	virtual bool IsEndRequest() const { return endRequest_; }

	// 3Dオブジェクト描画
	void PreDrawObject3d();

	// 2Dオブジェクト描画
	void PreDrawSprite();

	// 実行
	void Run();

	// ゲッター
	DirectXCommon* GetDirectXCommon() const { return dxCommon.get(); }

	//SpriteCommon* GetSpriteCommon() const { return spriteCommon.get(); }

	WinApp* GetWinApp() const { return winApp.get(); }
protected:
	// メンバ変数
	// ポインタ
	unique_ptr<WinApp> winApp = nullptr;

	unique_ptr<DirectXCommon> dxCommon = nullptr;

	unique_ptr<SrvManager> srvManager = nullptr;

	unique_ptr < Camera> camera = make_unique<Camera>();

	unique_ptr < ImGuiManager> imGuiManager = make_unique<ImGuiManager>();

	bool endRequest_ = false;

	std::unique_ptr<SceneManager> sceneManager_ = nullptr;

};


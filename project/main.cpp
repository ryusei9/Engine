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
#include <DierctXGame/application/scene/GameScene.h>
#include <Normalize.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")


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


// std::stringを受け取る関数
void Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	// COMの初期化
	CoInitializeEx(0, COINIT_MULTITHREADED);

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
	dxCommon = DirectXCommon::GetInstance();
	dxCommon->Initialize(winApp);

	// SRVマネージャの初期化
	SrvManager* srvManager = nullptr;

	srvManager = new SrvManager();
	srvManager->Initialize(dxCommon);

	// テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(dxCommon, srvManager);

	// テクスチャを事前にロード
	TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");
	TextureManager::GetInstance()->LoadTexture("resources/monsterBall.png");
	TextureManager::GetInstance()->LoadTexture("resources/mori.png");
	TextureManager::GetInstance()->LoadTexture("resources/title.png");
	TextureManager::GetInstance()->LoadTexture("resources/tutorial.png");

	////////////////////////
	// input
	////////////////////////
	// ポインタ
	Input* input = nullptr;

	// 入力の初期化
	input = Input::GetInstance();
	input->Initialize(winApp);

	
	// 3Dモデルマネージャの初期化
	ModelManager::GetInstance()->Initialize(dxCommon);

	// .objファイルからモデルを読み込む
	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("axis.obj");
	ModelManager::GetInstance()->LoadModel("mori.obj");
	ModelManager::GetInstance()->LoadModel("player.obj");
	ModelManager::GetInstance()->LoadModel("player_bullet.obj");
	ModelManager::GetInstance()->LoadModel("enemy_bullet.obj");
	ModelManager::GetInstance()->LoadModel("sky_sphere.obj");
	//ModelManager::GetInstance()->LoadModel("title.obj");
	ModelManager::GetInstance()->LoadModel("titleGuide.obj");
	ModelManager::GetInstance()->LoadModel("GAMEOVER.obj");
	ModelManager::GetInstance()->LoadModel("GAMECLEAR.obj");
	ModelManager::GetInstance()->LoadModel("tutorial.obj");

	ImGuiManager* imGuiManager = new ImGuiManager();

	//// XAudio2の初期化
	//IXAudio2* xAudio2 = nullptr;
	//IXAudio2MasteringVoice* masterVoice = nullptr;

	//HRESULT result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);

	//// マスターボイスの作成
	//result = xAudio2->CreateMasteringVoice(&masterVoice);
	Audio::GetInstance()->Initialize();

	// サウンドデータの読み込み
	SoundData soundData1 = Audio::GetInstance()->SoundLoadWave("resources/Alarm01.wav");
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
	std::unique_ptr<GameScene> gameScene_ = std::make_unique<GameScene>();
	gameScene_->Initialize();



	// 音声再生
	Audio::GetInstance()->SoundPlayWave(soundData1);
	

	imGuiManager->Initialize(winApp, dxCommon);

	MSG msg{};
	// ウィンドウの×ボタンが押されるまでループ
	while (true) {
		// Windowsのメッセージ処理
		if (winApp->ProcessMessage()) {
			// ゲームループを抜ける
			break;
		}
		gameScene_->Update();
		imGuiManager->Begin();
		
		gameScene_->DrawImGui();
    
		imGuiManager->End();
		/*ImGui::Render();*/
		/////////////////////
		//// コマンドをキック
		/////////////////////
		
		// 描画前処理
		dxCommon->PreDraw();


		srvManager->PreDraw();

		gameScene_->Draw();

		imGuiManager->Draw();
		
		dxCommon->PostDraw();


	}

	////////////////////
	// 解放処理
	////////////////////


	for (uint32_t i = 0; i < SrvManager::kMaxSRVCount; i++) {
		// 使われているSRVインデックスを解放
		srvManager->Free(i);
	}
	delete srvManager;
	
	Audio::GetInstance()->SoundUnload(&soundData1);
	Audio::GetInstance()->Finalize();

	
	// WindowsAPIの終了処理
	winApp->Finalize();
	delete winApp;
	
	CloseHandle(dxCommon->GetFenceEvent());
	

	// テクスチャマネージャの終了
	TextureManager::GetInstance()->Finalize();
	// 3Dモデルマネージャの終了
	ModelManager::GetInstance()->Finalize();

	imGuiManager->Finalize();
	delete imGuiManager;
	


	return 0;
}
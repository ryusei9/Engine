#include "SRFramework.h"
#include "TitleScene.h"

void SRFramework::Initialize()
{
	// COMの初期化
	CoInitializeEx(0, COINIT_MULTITHREADED);



	// WindowsAPIの初期化
	winApp = make_unique< WinApp>();
	winApp->Initialize();



	// DirectXの初期化
	dxCommon = make_unique<DirectXCommon>();
	dxCommon->Initialize(winApp.get());


	// SRVマネージャの初期化
	srvManager = make_unique<SrvManager>();
	srvManager->Initialize(dxCommon.get());

	// テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(dxCommon.get(), srvManager.get());

	// テクスチャを事前にロード
	TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");
	TextureManager::GetInstance()->LoadTexture("resources/monsterBall.png");
	TextureManager::GetInstance()->LoadTexture("resources/mori.png");
	TextureManager::GetInstance()->LoadTexture("resources/mori_Red.png");
	TextureManager::GetInstance()->LoadTexture("resources/circle2.png");
	
	// スプライト共通部の初期化
	
	SpriteCommon::GetInstance()->Initialize(dxCommon.get());


	// 3Dオブジェクト共通部の初期化
	Object3dCommon::GetInstance()->Initialize(dxCommon.get());


	// 3Dモデルマネージャの初期化
	ModelManager::GetInstance()->Initialize(dxCommon.get());

	// .objファイルからモデルを読み込む
	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("axis.obj");


	camera->SetRotate({ 0.0f,0.0f,0.0f });
	camera->SetTranslate({ 0.0f,0.0f,-10.0f });
	Object3dCommon::GetInstance()->SetDefaultCamera(camera.get());

#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(dxCommon.get()->GetDevice()->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
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

	imGuiManager->Initialize(winApp.get(), dxCommon.get());

	// シーンマネージャの初期化
	sceneManager_ = std::make_unique<SceneManager>();
	sceneManager_->Initialize(dxCommon.get(), winApp.get());

	/*------パーティクルマネージャの初期化------*/
	ParticleManager::GetInstance()->Initialize(dxCommon.get(), srvManager.get(),camera.get());
}

void SRFramework::Finelize()
{
	for (uint32_t i = 0; i < SrvManager::kMaxSRVCount; i++) {
		// 使われているSRVインデックスを解放
		srvManager->Free(i);
	}
	
	
	// WindowsAPIの終了処理
	winApp->Finalize();
	

	CloseHandle(dxCommon.get()->GetFenceEvent());
	

	// テクスチャマネージャの終了
	TextureManager::GetInstance()->Finalize();
	// 3Dモデルマネージャの終了
	ModelManager::GetInstance()->Finalize();

	// パーティクルマネージャの終了
	ParticleManager::GetInstance()->Finalize();
	
}

void SRFramework::Update()
{
	MSG msg{};
	// ウィンドウの×ボタンが押されるまでループ

	// Windowsのメッセージ処理
	if (winApp->ProcessMessage()) {
		// ゲームループを抜ける
		endRequest_ = true;
	}
	// シーンマネージャの更新
	sceneManager_->Update();
	camera->Update();
	// パーティクルマネージャの更新
	ParticleManager::GetInstance()->Update();
}

void SRFramework::PreDraw()
{
	// 描画前処理
	dxCommon.get()->PreDraw();

	srvManager->PreDraw();
	
}

void SRFramework::PostDraw()
{
	dxCommon.get()->PostDraw();
}

void SRFramework::PreDrawObject3d()
{
	// 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックコマンドを積む
	//Object3dCommon::GetInstance()->DrawSettings();
}

void SRFramework::PreDrawSprite()
{
	// Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
	//spriteCommon->DrawSettings();

	
}

void SRFramework::Run()
{
	// ゲームの初期化
	Initialize();
	while (true) {
		// メインループ
		Update();
		// ゲームループを抜ける
		if (IsEndRequest()) {
			break;
		}
		// 描画
		Draw();
	}
	// ゲームの終了
	Finelize();
}

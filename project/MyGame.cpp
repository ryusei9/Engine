#include "MyGame.h"


void MyGame::Initialize()
{
	// COMの初期化
	CoInitializeEx(0, COINIT_MULTITHREADED);



	// WindowsAPIの初期化
	winApp = new WinApp();
	winApp->Initialize();



	// DirectXの初期化
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);


	// SRVマネージャの初期化
	srvManager = new SrvManager();
	srvManager->Initialize(dxCommon);

	// テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(dxCommon, srvManager);

	// テクスチャを事前にロード
	TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");
	TextureManager::GetInstance()->LoadTexture("resources/monsterBall.png");
	TextureManager::GetInstance()->LoadTexture("resources/mori.png");
	////////////////////////
	// input
	////////////////////////


	// 入力の初期化
	input = new Input();
	input->Initialize(winApp);



	// スプライト共通部の初期化
	spriteCommon = new SpriteCommon;
	spriteCommon->Initialize(dxCommon);


	// 3Dオブジェクト共通部の初期化
	object3dCommon = new Object3dCommon;
	object3dCommon->Initialize(dxCommon);


	// 3Dモデルマネージャの初期化
	ModelManager::GetInstance()->Initialize(dxCommon);

	// .objファイルからモデルを読み込む
	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("axis.obj");


	camera->SetRotate({ 0.0f,0.0f,0.0f });
	camera->SetTranslate({ 0.0f,0.0f,-10.0f });
	object3dCommon->SetDefaultCamera(camera);



	Audio::GetInstance()->Initialize();

	// サウンドデータの読み込み
	soundData1 = Audio::GetInstance()->SoundLoadWave("resources/Alarm01.wav");

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








	sprite->Initialize(spriteCommon, dxCommon, "resources/mori.png");


	// 音声再生
	Audio::GetInstance()->SoundPlayWave(soundData1);


	imGuiManager->Initialize(winApp, dxCommon);
}

void MyGame::Finelize()
{
	for (uint32_t i = 0; i < SrvManager::kMaxSRVCount; i++) {
		// 使われているSRVインデックスを解放
		srvManager->Free(i);
	}
	delete srvManager;
	delete input;
	delete spriteCommon;

	delete object3dCommon;

	Audio::GetInstance()->SoundUnload(&soundData1);
	Audio::GetInstance()->Finalize();

	// WindowsAPIの終了処理
	winApp->Finalize();
	delete winApp;

	CloseHandle(dxCommon->GetFenceEvent());
	delete dxCommon;

	// テクスチャマネージャの終了
	TextureManager::GetInstance()->Finalize();
	// 3Dモデルマネージャの終了
	ModelManager::GetInstance()->Finalize();

	imGuiManager->Finalize();
	delete imGuiManager;
}

void MyGame::Update()
{
	MSG msg{};
	// ウィンドウの×ボタンが押されるまでループ

	// Windowsのメッセージ処理
	if (winApp->ProcessMessage()) {
		// ゲームループを抜ける
		endRequest_ = true;
	}

	imGuiManager->Begin();
#ifdef _DEBUG
	// 次に作成されるウィンドウのサイズを設定
	ImGui::SetNextWindowSize(ImVec2(500, 100), ImGuiCond_FirstUseEver);

	ImGui::Begin("sprite");
	ImGui::SliderFloat2("sprite.translate", &spritePosition.x, 0.0f, 1200.0f, "%0.1f");
	ImGui::End();
#endif
	// ゲームの処理
	// 入力の更新
	input->Update();
	// 操作
	if (input->PushKey(DIK_RIGHTARROW)) {
		cameraTransform.translate.x -= 0.1f;

	}
	if (input->PushKey(DIK_LEFTARROW)) {
		cameraTransform.translate.x += 0.1f;
	}
	camera->SetTranslate(cameraTransform.translate);
	camera->Update();

	sprite->Update();
	sprite->SetPosition(spritePosition);


	imGuiManager->End();

}

void MyGame::Draw()
{
	// 描画前処理
	dxCommon->PreDraw();


	srvManager->PreDraw();

	// 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックコマンドを積む
	object3dCommon->DrawSettings();

	// Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
	spriteCommon->DrawSettings();

	sprite->Draw();

	imGuiManager->Draw();

	dxCommon->PostDraw();
}

#include "SRFramework.h"
#include "TitleScene.h"
#include <GetNowTimeInSeconds.h>

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
	
	// スプライト共通部の初期化
	
	SpriteCommon::GetInstance()->Initialize(dxCommon.get());


	// 3Dオブジェクト共通部の初期化
	Object3dCommon::GetInstance()->Initialize(dxCommon.get(),srvManager.get());


	// 3Dモデルマネージャの初期化
	ModelManager::GetInstance()->Initialize(dxCommon.get());

	// .objファイルからモデルを読み込む
	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("axis.obj");


	camera->SetRotate({ 0.1f,0.0f,0.0f });
	camera->SetTranslate({ 0.0f,5.0f,-30.0f });
	Object3dCommon::GetInstance()->SetDefaultCamera(camera.get());

	dxCommon->CreateDepthResource(camera.get());

	dxCommon->CreateMaskSRVDescriptorHeap();

	postEffect_ = std::make_unique<NoisePostEffect>();
	postEffect_->Initialize(dxCommon.get());

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

	postEffect_->SetTimeParams(GetNowTimeInSeconds());
	// パーティクルマネージャの更新
	ParticleManager::GetInstance()->Update();
}

void SRFramework::PreDraw()
{
	// --- ここでの深度バッファ状態 ---
   // 前フレームのポストエフェクト後なので
   // [D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE]（SRV用）になっている

	// 深度バッファを「書き込み用」に遷移
	//dxCommon->TransitionDepthBufferToWrite();

	// --- ここでの深度バッファ状態 ---
	// [D3D12_RESOURCE_STATE_DEPTH_WRITE]（書き込み
	// 描画前処理
	dxCommon.get()->PreDraw();

	srvManager->PreDraw();
	
}

void SRFramework::PostDraw()
{
	dxCommon.get()->PostDraw();
}

void SRFramework::PrePostEffect()
{
	// --- ここでの深度バッファ状態 ---
	// [D3D12_RESOURCE_STATE_DEPTH_WRITE]（書き込み
	dxCommon.get()->PreRenderScene();
	postEffect_->PreRender();
}

void SRFramework::DrawPostEffect()
{
	// --- ここでの深度バッファ状態 ---
	// [D3D12_RESOURCE_STATE_DEPTH_WRITE]（書き込み用）

	// 深度バッファを「SRV用」に遷移
	dxCommon->TransitionDepthBufferToSRV();

	// --- ここでの深度バッファ状態 ---
   // [D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE]（SRV用）


	dxCommon.get()->TransitionRenderTextureToShaderResource();
	dxCommon.get()->DrawRenderTexture();
	dxCommon.get()->TransitionRenderTextureToRenderTarget();
	postEffect_->TransitionRenderTextureToShaderResource();
	postEffect_->Draw();
	postEffect_->TransitionRenderTextureToRenderTarget();

	// --- ここでの深度バッファ状態 ---
    // [D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE]（SRV用）のまま
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

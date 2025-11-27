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
	DirectXCommon::GetInstance()->Initialize(winApp.get());


	// SRVマネージャの初期化
	srvManager = make_unique<SrvManager>();
	srvManager->Initialize();

	// テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(srvManager.get());

	// テクスチャを事前にロード
	TextureManager::GetInstance()->LoadTexture("resources/uvChecker.png");
	TextureManager::GetInstance()->LoadTexture("resources/monsterBall.png");
	TextureManager::GetInstance()->LoadTexture("resources/mori.png");
	TextureManager::GetInstance()->LoadTexture("resources/mori_Red.png");
	TextureManager::GetInstance()->LoadTexture("resources/circle2.png");
	TextureManager::GetInstance()->LoadTexture("resources/gradationLine.png");
	TextureManager::GetInstance()->LoadTexture("resources/player.png");
	TextureManager::GetInstance()->LoadTexture("resources/Boss.png");
	TextureManager::GetInstance()->LoadTexture("resources/player_bullet.png");
	TextureManager::GetInstance()->LoadTexture("resources/white.png");
	TextureManager::GetInstance()->LoadTexture("resources/backGround.png");
	TextureManager::GetInstance()->LoadTexture("resources/fadeWhite.png");
	TextureManager::GetInstance()->LoadTexture("resources/BackToTitle.png");
	TextureManager::GetInstance()->LoadTexture("resources/Black.png");
	TextureManager::GetInstance()->LoadTexture("resources/boost.png");
	
	// スプライト共通部の初期化
	
	SpriteCommon::GetInstance()->Initialize(srvManager.get());


	// 3Dオブジェクト共通部の初期化
	Object3dCommon::GetInstance()->Initialize(srvManager.get());


	// 3Dモデルマネージャの初期化
	ModelManager::GetInstance()->Initialize();

	// .objファイルからモデルを読み込む
	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("axis.obj");
	ModelManager::GetInstance()->LoadModel("BackToTitle.obj"); // ←追加
	ModelManager::GetInstance()->LoadModel("testCube.obj");
	ModelManager::GetInstance()->LoadModel("testGround.obj");
	// 弾用モデルを事前にロード
	ModelManager::GetInstance()->LoadModel("player_bullet.obj"); // ←追加



	Input::GetInstance()->Initialize(winApp.get());

	camera->SetRotate({ 0.1f,0.0f,0.0f });
	camera->SetTranslate({ 0.0f,1.0f,-10.0f });
	Object3dCommon::GetInstance()->SetDefaultCamera(camera.get());

	dxCommon = DirectXCommon::GetInstance();

	dxCommon->CreateDepthResource(camera.get());

	dxCommon->CreateMaskSRVDescriptorHeap();

	// 各種ポストエフェクトの初期化
	noisePostEffect_ = std::make_unique<NoisePostEffect>();
	noisePostEffect_->Initialize(dxCommon);

	grayscalePostEffect_ = std::make_unique<GrayscalePostEffect>();
	grayscalePostEffect_->Initialize(dxCommon);

	// ポストエフェクトマネージャにポストエフェクトを追加
	postEffectManager_ = std::make_unique<PostEffectManager>();
	postEffectManager_->Initialize(dxCommon);
	postEffectManager_->AddEffect(std::move(noisePostEffect_));
	postEffectManager_->AddEffect(std::move(grayscalePostEffect_));

	postEffectManager_->SetEffectEnabled(0, false);
	postEffectManager_->SetEffectEnabled(1, false);

	imGuiManager->Initialize(winApp.get());

	/*------パーティクルマネージャの初期化------*/
	ParticleManager::GetInstance()->Initialize(srvManager.get(), camera.get());

	// シーンマネージャの初期化
	sceneManager_ = std::make_unique<SceneManager>();
	sceneManager_->Initialize(winApp.get());

	/*------パーティクルマネージャの初期化------*/
	ParticleManager::GetInstance()->Initialize(srvManager.get(),camera.get());
}

void SRFramework::Finelize()
{
	for (uint32_t i = 0; i < SrvManager::kMaxSRVCount; i++) {
		// 使われているSRVインデックスを解放
		srvManager->Free(i);
	}
	
	
	// WindowsAPIの終了処理
	winApp->Finalize();
	

	CloseHandle(DirectXCommon::GetInstance()->GetFenceEvent());
	

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

	postEffectManager_->SetTimeParams(GetNowTimeInSeconds());
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
	DirectXCommon::GetInstance()->PreDraw();

	srvManager->PreDraw();
	
}

void SRFramework::PostDraw()
{
	DirectXCommon::GetInstance()->PostDraw();
}

void SRFramework::PrePostEffect()
{
	// [D3D12_RESOURCE_STATE_DEPTH_WRITE]（書き込み
	dxCommon->PreRenderScene();
	postEffectManager_->PreRenderAll();
}

void SRFramework::DrawPostEffect()
{
	// 深度バッファを「SRV用」に遷移
	dxCommon->TransitionDepthBufferToSRV();

	dxCommon->TransitionRenderTextureToShaderResource();
	dxCommon->DrawRenderTexture();
	dxCommon->TransitionRenderTextureToRenderTarget();
	postEffectManager_->PreBarrierAll();
	postEffectManager_->DrawAll();
	postEffectManager_->PostBarrierAll();

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

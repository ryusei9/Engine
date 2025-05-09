#include "DirectXCommon.h"
#include <cassert>
#include "Logger.h"
#include "StringUtility.h"
#include <format>
#include <dxcapi.h>
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/DirectXTex/d3dx12.h"
#include <thread>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

using namespace Microsoft::WRL;

const uint32_t DirectXCommon::kMaxSRVCount = 512;
void DirectXCommon::Initialize(WinApp* winApp)
{
	// FPS固定初期化
	initializeFixFPS();
	// NULL検出
	assert(winApp);

	// メンバ変数に記録
	winApp_ = winApp;

	// デバイスの生成
	DeviceInitialize();
	// コマンド関係の初期化
	CommandInitialize();
	// スワップチェーンの生成
	SwapChain();
	// 深度バッファの生成
	CreateBuffer(WinApp::kClientWidth, WinApp::kClientHeight);
	// 各種デスクリプタヒープの生成
	DescriptorHeap();
	
	// レンダーターゲットビュー
	RenderTargetView();

	CreateRenderTexture();
	// 深度ステンシルビューの初期化
	DepthStencilViewInitialize();
	// フェンスの初期化
	FenceInitialize();
	// ビューポート矩形の初期化
	ViewPortInitialize();
	// シザリング矩形の初期化
	ScissorRectInitialize();
	// DXCコンパイラの初期化
	CreateDXCCompiler();
	// ImGuiの初期化
	//ImGuiInitialize();
}

void DirectXCommon::DeviceInitialize()
{
	//HRESULT hr;
	////////////////////
	// デバッグレイヤー
	////////////////////

#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		// デバッグレイヤーを有効化する
		debugController->EnableDebugLayer();
		// さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif
	//////////////////////
	// DirectX12の初期化
	//////////////////////

	// DXGIファクトリーの生成
	dxgiFactory = nullptr;
	// 関数が成功したかどうか
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	// エラー識別
	assert(SUCCEEDED(hr));
	// 使用するアダプタ用の変数
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;
	// 良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
		DXGI_ERROR_NOT_FOUND; ++i) {
		// アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));
		// ソフトウェアアダプタでなければ使用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			// 採用したアダプタの情報をログに出力
			// その際std::wstringをstd::stringに変換
			Logger::Log(StringUtility::ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		// ソフトウェアアダプタは無視
		useAdapter = nullptr;
	}
	// 適切なアダプタが無いときは起動できない
	assert(useAdapter != nullptr);

	device = nullptr;
	// 機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	}; const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
	// 高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		// 採用したアダプターでデバイスを生成
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
		// 指定した機能レベルでデバイスが生成できたかを確認
		if (SUCCEEDED(hr)) {
			// 生成できたのでログ出力をおっこなってループを抜ける
			Logger::Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	// デバイスの生成が上手くいかなかったので起動できない
	assert(device != nullptr);
	// 初期化完了のログを出す
	Logger::Log("Complete create D3D12Device!!!\n");
#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
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
}

void DirectXCommon::CommandInitialize()
{
	HRESULT hr;

	// コマンドアロケータを生成する
	commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	// コマンドアロケータの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	// コマンドリストを生成する
	commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr,
		IID_PPV_ARGS(&commandList));
	// コマンドリストの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	// コマンドキューを生成する
	commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc,
		IID_PPV_ARGS(&commandQueue));
	// コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));
}

void DirectXCommon::SwapChain()
{
	/////////////////////////
	// スワップチェーンを生成する
	/////////////////////////
	HRESULT hr;
	swapChain = nullptr;
	/*DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};*/
	// 画面の幅
	swapChainDesc.Width = WinApp::kClientWidth;
	// 画面の高さ
	swapChainDesc.Height = WinApp::kClientHeight;
	// 色の形式
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	// マルチサンプルしない
	swapChainDesc.SampleDesc.Count = 1;
	// 描画のターゲットとして利用する
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	// ダブルバッファ
	swapChainDesc.BufferCount = backBufferCount;
	// モニタにうつしたら、中身を破棄
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	// コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winApp_->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));
}

void DirectXCommon::CreateBuffer(int32_t width, int32_t height)
{
	// 生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	// Textureの幅
	resourceDesc.Width = width;
	// Textureの高さ
	resourceDesc.Height = height;
	// mipmapの数
	resourceDesc.MipLevels = 1;
	// 奥行き or 配列Textureの配列数
	resourceDesc.DepthOrArraySize = 1;
	// DepthStencilとして利用可能なフォーマット
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// サンプリングカウント。1固定
	resourceDesc.SampleDesc.Count = 1;
	// 2次元
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	// DepthStencilとして使う通知
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	// VRAM上に作る
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// 深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	// 1.0f(最大値)でクリア
	depthClearValue.DepthStencil.Depth = 1.0f;
	// フォーマット。Resourceと合わせる
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Resourceの生成
	HRESULT hr = device->CreateCommittedResource(
		// Heapの設定
		&heapProperties,
		// Heapの特殊な設定。特になし
		D3D12_HEAP_FLAG_NONE,
		// Resourceの設定
		&resourceDesc,
		// 深度値を書き込む状態にしておく
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		// Clear最適値
		&depthClearValue,
		// 作成するResourceポインタへのポインタ
		IID_PPV_ARGS(&depthStencilResource));
	assert(SUCCEEDED(hr));
}

void DirectXCommon::DescriptorHeap()
{
	// DescriptorSizeを取得しておく
	descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	// ディスクリプタヒープの生成
	// ディスクリプタの数は2。RTVはshader内で触るものではないなので、shaderVisibleはfalse
	rtvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);

	// ディスクリプタの数は128。SRVはshader内で触るものなので、shaderVisibleはtrue
	srvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);

	// DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないので、ShaderVisibleはfalse
	dsvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	CreateCBVSRVUAVDescriptorHeap(device.Get());
	// SRVディスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1; // 必要なディスクリプタの数
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	// SRVディスクリプタヒープの作成
	HRESULT hr = device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap));
	assert(SUCCEEDED(hr));


	//// DSVの設定
	//D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	//// Format。基本的にはResourceに合わせる
	//dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//// 2dTexture
	//dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	//// DSVHeapの先頭にDSVをつくる
	//device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	//// SwapChainからResourceを引っ張ってくる
	//Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2] = { nullptr };
	//hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));

	//// うまく取得できなければ起動できない
	//assert(SUCCEEDED(hr));
	//hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	//assert(SUCCEEDED(hr));
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
{
	// ディスクリプタヒープの生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	// レンダーターゲットビュー用
	descriptorHeapDesc.Type = heapType;
	// ディスクリプタの数
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr;
	hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	// ディスクリプタヒープが作れなかったので起動できない
	assert(SUCCEEDED(hr));
	return descriptorHeap;

}

void DirectXCommon::RenderTargetView()
{
	HRESULT hr;
	// SwapChainからResourceを引っ張ってくる
	//swapChainResources[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));

	// うまく取得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));

	/////////////////
	// RTVの設定
	/////////////////

	/*D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};*/
	// 出力結果をSRGBに変換して書き込む
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 2dテクスチャとして書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	// ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = GetCPUDescriptorHandle(rtvDescriptorHeap.Get(), descriptorSizeRTV, 0);
	//// RTVを2つ作るのでディスクリプタを2つ用意
	//D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	// まず1つ目を作る。1つ目は最初のところに作る。
	// 裏表の2つ分
	//まず一つ目は最初の所につくる。作る場所をこちらで指定してあげる必要がある
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);
	// 2つ目のディスクリプタハンドルを得る
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// 2つ目を作る
	device->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVCPUDescriptorHandle(uint32_t index)
{
	return GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVGPUDescriptorHandle(uint32_t index)
{
	return GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, index);
}

void DirectXCommon::DepthStencilViewInitialize()
{
	// DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	// DepthStencilTextureをウィンドウのサイズで作成
	//Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = CreateDepthStencilTextureResource(device, WinApp::kClientWidth, WinApp::kClientHeight);
	// Format。基本的にはResourceに合わせる
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 2dTexture
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	// DSVHeapの先頭にDSVをつくる
	device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height)
{
	// 生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	// Textureの幅
	resourceDesc.Width = width;
	// Textureの高さ
	resourceDesc.Height = height;
	// mipmapの数
	resourceDesc.MipLevels = 1;
	// 奥行き or 配列Textureの配列数
	resourceDesc.DepthOrArraySize = 1;
	// DepthStencilとして利用可能なフォーマット
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// サンプリングカウント。1固定
	resourceDesc.SampleDesc.Count = 1;
	// 2次元
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	// DepthStencilとして使う通知
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	// VRAM上に作る
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// 深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	// 1.0f(最大値)でクリア
	depthClearValue.DepthStencil.Depth = 1.0f;
	// フォーマット。Resourceと合わせる
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		// Heapの設定
		&heapProperties,
		// Heapの特殊な設定。特になし
		D3D12_HEAP_FLAG_NONE,
		// Resourceの設定
		&resourceDesc,
		// 深度値くぉ書き込む状態にしておく
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		// Clear最適値
		&depthClearValue,
		// 作成するResourceポインタへのポインタ
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

void DirectXCommon::FenceInitialize()
{
	HRESULT hr;

	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	if (FAILED(hr)) {
		Logger::Log("Failed to create fence.");
		throw std::runtime_error("Fence creation failed.");
	}

	//// Fenceのsignalを待つためのイベントを作成
	fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (fenceEvent == nullptr) {
		Logger::Log("Failed to create fence event.");
		throw std::runtime_error("Fence event creation failed.");
	}
}

void DirectXCommon::ViewPortInitialize()
{
	//// ビューポート
	//D3D12_VIEWPORT viewport{};

	// クライアント領域のサイズと一緒にして画面全体に表示
	viewport.Width = WinApp::kClientWidth;
	viewport.Height = WinApp::kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}

void DirectXCommon::ScissorRectInitialize()
{
	//// シザー矩形
	//D3D12_RECT scissorRect{};

	// 基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect.left = 0;
	scissorRect.right = WinApp::kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = WinApp::kClientHeight;
}

void DirectXCommon::CreateDXCCompiler()
{
	HRESULT hr;

	//////////////////////////
	// dxcCompilerを初期化
	//////////////////////////

	/*IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;*/

	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	// includeに対応するための設定
	/*IDxcIncludeHandler* includeHandler = nullptr;*/
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));
}

void DirectXCommon::ImGuiInitialize()
{
	/////////////////////
	// ImGuiの初期化
	/////////////////////
	/*IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp_->GetHwnd());
	ImGui_ImplDX12_Init(device.Get(),
		swapChainDesc.BufferCount,
		rtvDesc.Format,
		srvDescriptorHeap.Get(),
		GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 0),
		GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 0));*/
}

/*------RenderTextureの描画------*/
void DirectXCommon::PreRenderScene()
{
	//CreateRenderTexture();
	// TransitionBarrierの設定
	//ChengeBarrier();

	// RenderTargetViewのハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = renderTextureRtvHeap->GetCPUDescriptorHandleForHeapStart();
	// DepthStencilViewのハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	// 描画先をRenderTextureに変更
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// RenderTextureをクリア
	commandList->ClearRenderTargetView(rtvHandle, reinterpret_cast<const FLOAT*>(&kRenderTargetClearValue), 0, nullptr);

	// DepthStencilをクリア
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// ビューポートの設定
	// ビューポートとシザーを新しく作ってみる
	commandList->RSSetViewports(1, &viewport);

	// シザーレクトの設定
	commandList->RSSetScissorRects(1, &scissorRect);
}

/*------スワップチェインの描画------*/
void DirectXCommon::PreDraw()
{
	// TransitionBarrierの設定
	ChengeBarrier();


	// 描画先のRTVとDSVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetDSVCPUDescriptorHandle(0);

	// 描画先のRTVを設定
	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);

	// 指定した色で画面全体をクリアする
	// 青っぽい色。RGBA
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
	
	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

	// 指定した深度で画面全体をクリアする
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


	/// 三角形の描画
	// Viewportを設定
	commandList->RSSetViewports(1, &viewport);
	// Scissorを設定
	commandList->RSSetScissorRects(1, &scissorRect);
}

void DirectXCommon::PostDraw()
{
	HRESULT hr;
	// これから書き込むバックバッファのインデックスを取得する
	UINT bbIndex = swapChain->GetCurrentBackBufferIndex();
	// 実際のcommandListのImGuiの描画コマンドを積む
	//ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());

	// 画面に描く処理はすべて終わり、画面に映すので、状態を遷移
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	// TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);

	// コマンドリストの内容を確定させる。すべてのコマンドを積んでcloseすること
	hr = commandList->Close();
	assert(SUCCEEDED(hr));

	// GPUにコマンドリストの実行を行わせる
	ID3D12CommandList* commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(1, commandLists);

	// GPUとOSに画面の交換を行うよう通知する
	swapChain->Present(1, 0);

	// Fenceの値を更新
	fenceValue++;
	// GPUがここまでたどり着いた時に、Fenceの値を指定した値に代入するようにsignalを送る
	commandQueue->Signal(fence.Get(), ++fenceValue);

	// Fenceの値が指定したsignal値にたどり着いているか確認する
	// GetCompletedValueの初期値はFence作成時に
	if (fence->GetCompletedValue() != fenceValue) {
		// 指定したsignalにたどりついていないので、たどり着くまで待つようにイベントを設定する
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		// イベント待つ
		WaitForSingleObject(fenceEvent, INFINITE);
		UpdateFixFPS();
	}

	// 次のフレーム用のコマンドリストを準備
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));
}

void DirectXCommon::ChengeBarrier()
{
	// これから書き込むバックバッファのインデックスを取得する
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	// 今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	// Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	// バリアを張る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();

	// 遷移前(現在)のResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	// 遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	// TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);
}

Microsoft::WRL::ComPtr<IDxcBlob> DirectXCommon::CompileShader(const std::wstring& filePath, const wchar_t* profile)
{
	// これからシェーダーをコンパイルする旨をログに出す
	Logger::Log(StringUtility::ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));
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
		Logger::Log(shaderError->GetStringPointer());
		// 警告・エラーは駄目です
		assert(false);
	}
	// compile結果を受け取って渡す
	// コンパイル結果から実行用のバイナリ部分を取得
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	// 成功したログを出す
	Logger::Log(StringUtility::ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));
	// もう使わないリソースを解放
	shaderSource->Release();
	shaderResult->Release();
	// 実行用のバイナリを返却
	return shaderBlob;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateBufferResource(size_t sizeInBytes)
{
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
	return resource.Get();
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateTextureResource(const DirectX::TexMetadata& metadata)
{
	// metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	// Textureの幅
	resourceDesc.Width = UINT(metadata.width);
	// Textureの高さ
	resourceDesc.Height = UINT(metadata.height);
	// mipmapの数
	resourceDesc.MipLevels = UINT(metadata.mipLevels);
	// 奥行き or 配列Textureの配列数
	resourceDesc.DepthOrArraySize = UINT(metadata.arraySize);
	// TextureのFormat
	resourceDesc.Format = metadata.format;
	// サンプリングカウント。1固定
	resourceDesc.SampleDesc.Count = 1;
	// textureの次元数。普段使っているのは2次元
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	// 細かい設定を行う
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	// WriteBackポリシーでCPUアクセス可能
	// UNKNOWNを設定
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	// プロセッサの近くに配置
	// UNKNOWNを設定
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

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
		D3D12_RESOURCE_STATE_COPY_DEST,
		// Clear最適値。使わないのでnullptr
		nullptr,
		// 作成するResourceポインタへのポインタ
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

Microsoft::WRL::ComPtr <ID3D12Resource> DirectXCommon::UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages)
{
	std::vector<D3D12_SUBRESOURCE_DATA> subResources;
	DirectX::PrepareUpload(device.Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subResources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subResources.size()));

	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(intermediateSize);
	UpdateSubresources(commandList.Get(), texture.Get(), intermediateResource.Get(), 0, 0, UINT(subResources.size()), subResources.data());
	//Textureへの転送後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);

	return intermediateResource;
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}
D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetDSVCPUDescriptorHandle(uint32_t index)
{
	return GetCPUDescriptorHandle(dsvDescriptorHeap, descriptorSizeDSV, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetDSVGPUDescriptorHandle(uint32_t index)
{
	return GetGPUDescriptorHandle(dsvDescriptorHeap, descriptorSizeDSV, index);
}

void DirectXCommon::initializeFixFPS()
{
	// 現在時間を記録する
	reference_ = std::chrono::steady_clock::now();
}

void DirectXCommon::UpdateFixFPS()
{
	// 1/60秒ぴったりの時間
	const std::chrono::microseconds kMinTime(uint64_t(1000000.0f / 60.0f));
	// 1/60秒よりわずかに短い時間
	const std::chrono::microseconds kMinCheckTime(uint64_t(1000000.0f / 65.0f));

	// 現在時間を取得する
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

	// 前回記録からの経過時間を取得する
	std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

	// 1/60秒(よりわずかに短い時間)経っていない場合
	if (elapsed < kMinCheckTime) {
		// 1/60秒経過するまで微小なスリープを繰り返す
		while (std::chrono::steady_clock::now() - reference_ < kMinTime) {
			// 1マイクロスリープ
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}
	// 現在の時間を記録する
	reference_ = std::chrono::steady_clock::now();
}

DirectX::ScratchImage DirectXCommon::LoadTexture(const std::string& filePath)
{
	// テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);

	// ミップマップ付きのデータを渡す
	return mipImages;
}

void DirectXCommon::SyncCPUWithGPU()
{
	HRESULT hr;
	hr = commandList->Close();
	assert(SUCCEEDED(hr));



	//GPUにコマンドリストの実行行わせる
	ID3D12CommandList* commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(1, commandLists);



	fenceValue++;



	//GPUがここまでたどり着いた時に、Fenceの値を指定した値に代入するようにSignalをおくる
	commandQueue->Signal(fence.Get(), fenceValue);



	if (fence->GetCompletedValue() < fenceValue) {
		//
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		//イベント待つ
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	//FPS固定更新
	//UpdateFixFPS();

		//次のフレーム用のコマンドリストを準備
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));


	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));

}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateRenderTextureResoruce(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor)
{
	/*------生成するResourceの設定------*/
	D3D12_RESOURCE_DESC resourceDesc{};
	// Textureの幅
	resourceDesc.Width = width;
	// Textureの高さ
	resourceDesc.Height = height;
	// mipmapの数
	resourceDesc.MipLevels = 1;
	// 奥行き or 配列Textureの配列数
	resourceDesc.DepthOrArraySize = 1;
	// DepthStencilとして利用可能なフォーマット
	resourceDesc.Format = format;
	// サンプリングカウント。1固定
	resourceDesc.SampleDesc.Count = 1;
	// 2次元
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	// RenderTargetとして使う通知
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	/*------利用するHeapの設定------*/
	D3D12_HEAP_PROPERTIES heapProperties{};
	// VRAM上に作る
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	/*------RenderTarget用の設定------*/
	D3D12_CLEAR_VALUE renderTargetClearValue{};
	// フォーマット。Resourceと合わせる
	renderTargetClearValue.Format = format;
	// ClearColorを設定
	renderTargetClearValue.Color[0] = clearColor.x;
	renderTargetClearValue.Color[1] = clearColor.y;
	renderTargetClearValue.Color[2] = clearColor.z;
	renderTargetClearValue.Color[3] = clearColor.w;

	/*------Resourceの生成------*/
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		// Heapの設定
		&heapProperties,
		// Heapの特殊な設定。特になし
		D3D12_HEAP_FLAG_NONE,
		// Resourceの設定
		&resourceDesc,
		// 深度値を書き込む状態にしておく
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		// Clear最適値
		&renderTargetClearValue,
		// 作成するResourceポインタへのポインタ
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	// RenderTarget用のResourceを作成したので、Resourceを返す
	return resource;
}

void DirectXCommon::CreateRenderTexture()
{
	/*------RenderTextureのRTVを生成する------*/
	renderTexture = CreateRenderTextureResoruce(device, WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTargetClearValue);

	renderTextureRtvHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1, false);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = renderTextureRtvHeap->GetCPUDescriptorHandleForHeapStart();
	device->CreateRenderTargetView(renderTexture.Get(), &rtvDesc, handle);

	/*------RenderTextureのSRVを生成する------*/
	// SRVの設定。FormatはResourceに合わせる
	// SRVのFormat
	renderTextureSrvDesc.Format = renderTexture->GetDesc().Format;
	// SRVのShader4ComponentMapping
	renderTextureSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	// SRVのViewDimension
	renderTextureSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	// SRVのTexture2Dの設定
	renderTextureSrvDesc.Texture2D.MipLevels = 1;

	// SRVの作成
	device->CreateShaderResourceView(renderTexture.Get(), &renderTextureSrvDesc, srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void DirectXCommon::CreateCBVSRVUAVDescriptorHeap(ID3D12Device* device)
{
	// ディスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 100; // 必要なディスクリプタ数
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // シェーダーからアクセス可能にする
	heapDesc.NodeMask = 0;

	// ディスクリプタヒープを作成
	HRESULT hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&cbvSrvUavDescriptorHeap));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create CBV/SRV/UAV descriptor heap.");
	}
}

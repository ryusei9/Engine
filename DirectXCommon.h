#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "WinApp.h"
#include <array>
#include <dxcapi.h>
#include "externals/DirectXTex/DirectXTex.h"
#include "string"
#include "chrono"
// DirectX基盤
class DirectXCommon
{
public: // メンバ関数
	// 初期化
	void Initialize(WinApp* winApp);

	// デバイスの初期化
	void DeviceInitialize();

	// コマンド関連の初期化
	void CommandInitialize();

	// スワップチェーンの生成
	void SwapChain();

	// 深度バッファの生成
	void CreateBuffer(int32_t width, int32_t height);

	// 各種デスクリプタヒープの生成
	void DescriptorHeap();

	/// <summary>
	/// デスクリプタヒープを生成する
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible
	);

	// レンダーターゲットビューの初期化
	void RenderTargetView();

	/// <summary>
	/// SRVの指定番号のCPUデスクリプタハンドルを取得する
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);

	/// <summary>
	/// SRVの指定番号のGPUデスクリプタハンドルを取得する
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

	// 深度ステンシルビューの初期化
	void DepthStencilViewInitialize();

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);

	// フェンスの生成
	void FenceInitialize();

	// ビューポート矩形の初期化
	void ViewPortInitialize();

	// シザリング矩形の初期化
	void ScissorRectInitialize();

	// DXCコンパイラの生成
	void CreateDXCCompiler();

	// ImGuiの初期化
	void ImGuiInitialize();

	// 描画前処理
	void PreDraw();

	// 描画後処理
	void PostDraw();

	// CompileShader関数
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
// compilerするshaderファイルへのパス
		const std::wstring& filePath,
		// compilerに使用するprofile
		const wchar_t* profile
	);
	// BufferResourceの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	// DirectX12のTextureResourceを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metadata);

	// TextureResourceにデータを転送する
	Microsoft::WRL::ComPtr <ID3D12Resource> UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages);
	
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetDSVGPUDescriptorHandle(uint32_t index);

	// FPS固定初期化
	void initializeFixFPS();
	// FPS固定更新
	void UpdateFixFPS();

	// 記録時間
	std::chrono::steady_clock::time_point reference_;
	/// <summary>
	/// ゲッター
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const{ return device.Get(); }

	IDxcUtils* GetDxcUtils() { return dxcUtils; }

	IDxcCompiler3* GetDxcCompiler() { return dxcCompiler; }

	IDxcIncludeHandler* GetIncludeHandler() { return includeHandler; }

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetSRVDescriptorHeap() { return srvDescriptorHeap; }
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDSVDescriptorHeap() { return dsvDescriptorHeap; }

	uint32_t GetDescriptorSizeSRV() { return descriptorSizeSRV; }
	uint32_t GetDescriptorSizeDSV() { return descriptorSizeDSV; }

	Microsoft::WRL::ComPtr<IDXGISwapChain4> GetSwapChain() { return swapChain; }

	//std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> GetSwapChainResources(){return swapChainResources[0] }
	HANDLE GetFenceEvent() { return fenceEvent; }

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() const { return commandList.Get(); }

	D3D12_CPU_DESCRIPTOR_HANDLE(&GetRtvHandles())[2] {return rtvHandles; }

	UINT GetBackBufferIndex() { return backBufferIndex; }

private:
	// 関数

	/// <summary>
	/// 指定番号のCPUデスクリプタハンドルを取得する
	/// </summary>
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);

	/// <summary>
	/// 指定番号のGPUデスクリプタハンドルを取得する
	/// </summary>
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

	
	// DirectXTexを使ってTextureを読むためのLoadTexture関数
	static DirectX::ScratchImage LoadTexture(const std::string& filePath);

	// DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	// DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
	// WindowsAPI
	WinApp* winApp_ = nullptr;

	// スワップチェーン
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	// リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = nullptr;
	// 各種DescriptorSize
	uint32_t descriptorSizeRTV;
	uint32_t descriptorSizeSRV;
	uint32_t descriptorSizeDSV;

	// デスクリプタ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;

	// SwapChainからResourceを引っ張ってくる
	//Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2];

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	// RTVハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

	// コマンドアロケータ
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	// コマンドキュー
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;

	// スワップチェーンリソース
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;

	// フェンス生成
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	uint64_t fenceValue = 0;

	// Fenceのsignalを待つためのイベントを作成
	HANDLE fenceEvent;

	// ビューポート
	D3D12_VIEWPORT viewport{};

	// シザー矩形
	D3D12_RECT scissorRect{};

	// DXCの生成物
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	IDxcIncludeHandler* includeHandler = nullptr;

	// TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};

	UINT backBufferIndex;

};


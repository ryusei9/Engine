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
#include <Vector4.h>
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

	// RenderTextureに対してのSceneの描画
	void PreRenderScene();

	// 描画前処理
	void PreDraw();

	// 描画後処理
	void PostDraw();

	// バリアの設定
	void ChengeBarrier();

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
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);

	// TextureResourceにデータを転送する
	Microsoft::WRL::ComPtr <ID3D12Resource> UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages);
	
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetDSVGPUDescriptorHandle(uint32_t index);

	// FPS固定初期化
	void initializeFixFPS();
	// FPS固定更新
	void UpdateFixFPS();
	// CPUとGPUの同期を待つ
	void SyncCPUWithGPU();

	// RenderTextureの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTextureResoruce(Microsoft::WRL::ComPtr<ID3D12Device> device,uint32_t width,uint32_t height,DXGI_FORMAT format,const Vector4& clearColor);

	// RenderTextureを生成
	void CreateRenderTexture();

	// ディスクリプタヒープを作成する関数
	void CreateCBVSRVUAVDescriptorHeap(ID3D12Device* device);

	// RootSignatureの作成
	void CreateRootSignature();

	// PSOの作成
	void CreatePSO();

	// コピーの前に張る1のバリア
	void TransitionRenderTextureToShaderResource();

	// コピーの後に張る2のバリア
	void TransitionRenderTextureToRenderTarget();

	// RenderTextureを描画
	void DrawRenderTexture();
	
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

	// ディスクリプタヒープを取得する関数
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetCBVSRVUAVDescriptorHeap() const {return cbvSrvUavDescriptorHeap;}

	uint32_t GetDescriptorSizeSRV() { return descriptorSizeSRV; }
	uint32_t GetDescriptorSizeDSV() { return descriptorSizeDSV; }

	Microsoft::WRL::ComPtr<IDXGISwapChain4> GetSwapChain() { return swapChain; }

	//std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> GetSwapChainResources(){return swapChainResources[0] }
	HANDLE GetFenceEvent() { return fenceEvent; }

	ID3D12GraphicsCommandList* GetCommandList() const { return commandList.Get(); }

	D3D12_CPU_DESCRIPTOR_HANDLE(&GetRtvHandles())[2] {return rtvHandles; }

	UINT GetBackBufferIndex() { return backBufferIndex; }

	UINT GetBackBufferCount() const { return backBufferCount; }

	// 最大SRV数(最大テクスチャ枚数)
	static const uint32_t kMaxSRVCount;

	/// <summary>
	/// 指定番号のCPUデスクリプタハンドルを取得する
	/// </summary>
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);

	/// <summary>
	/// 指定番号のGPUデスクリプタハンドルを取得する
	/// </summary>
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

private:
	// 関数

	
	
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

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> cbvSrvUavDescriptorHeap;

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

	UINT backBufferCount = 2;

	// 今回は赤を設定する
	const Vector4 kRenderTargetClearValue = { 1.0f,0.0f,0.0f,1.0f };

	Microsoft::WRL::ComPtr<ID3D12Resource> renderTexture;

	D3D12_SHADER_RESOURCE_VIEW_DESC renderTextureSrvDesc{};

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> renderTextureRtvHeap;

	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};

	/// BlendStateの設定
	D3D12_RENDER_TARGET_BLEND_DESC blendDesc{};

	/// RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc{};

	// shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob;

	/// PixelShader
	// shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob;

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

	// グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

	D3D12_RESOURCE_STATES renderTextureState = D3D12_RESOURCE_STATE_RENDER_TARGET;
};


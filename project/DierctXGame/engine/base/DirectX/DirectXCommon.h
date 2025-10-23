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
#include <DepthMaterial.h>
#include <ResourceManager.h>

// 定数バッファ用データ構造体
struct DissolveParams
{
	float threshold;
	float edgeWidth;
	float edgeStrength;
	float edgeColor[3];
	float pad; // 16バイトアライメント用
};

// 時間用定数バッファ
struct TimeParams
{
	float time;
	float pad[3]; // 16バイトアライメント
};
/// <summary>
/// DirectX基盤
/// </summary>
class DirectXCommon
{
public: // メンバ関数
	// シングルトンインスタンス取得
	static DirectXCommon* GetInstance();
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

	// 深度ステンシルテクスチャリソースの生成
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
	
	// 深度ステンシルビューの指定番号のCPUデスクリプタハンドルを取得する
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUDescriptorHandle(uint32_t index);

	// 深度ステンシルビューの指定番号のGPUデスクリプタハンドルを取得する
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

	void CreateDepthSRVDescriptorHeap();

	// depth用のリソースの作成
	void CreateDepthResource(Camera* camera);

	// depth用のSRVを作成
	void TransitionDepthBufferToSRV();

	// depth用の書き込み状態に遷移
	void TransitionDepthBufferToWrite();

	// mask用のSRVディスクリプタヒープを作成
	void CreateMaskSRVDescriptorHeap();

	// dissolve用の定数バッファを作成
	void CreateDissolveParamBuffer();
	
	// 記録時間
	std::chrono::steady_clock::time_point reference_;
	/// <summary>
	/// ゲッター
	/// </summary>
	 
	// DirectX12デバイスを取得する関数
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const{ return device.Get(); }

	// コマンドキューを取得する関数
	IDxcUtils* GetDxcUtils() { return dxcUtils; }

	// DXCコンパイラを取得する関数
	IDxcCompiler3* GetDxcCompiler() { return dxcCompiler; }

	// DXCインクルードハンドラを取得する関数
	IDxcIncludeHandler* GetIncludeHandler() { return includeHandler; }

	// SRVデスクリプタヒープを取得する関数
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetSRVDescriptorHeap() { return srvDescriptorHeap; }

	// DSVデスクリプタヒープを取得する関数
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDSVDescriptorHeap() { return dsvDescriptorHeap; }

	// ディスクリプタヒープを取得する関数
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetCBVSRVUAVDescriptorHeap() const {return cbvSrvUavDescriptorHeap;}

	// デスクリプタサイズ取得関数
	uint32_t GetDescriptorSizeSRV() { return descriptorSizeSRV; }
	uint32_t GetDescriptorSizeDSV() { return descriptorSizeDSV; }

	// スワップチェーンを取得する関数
	Microsoft::WRL::ComPtr<IDXGISwapChain4> GetSwapChain() { return swapChain; }
	
	// フェンスを取得する関数
	HANDLE GetFenceEvent() { return fenceEvent; }

	// コマンドリストを取得する関数
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList.Get(); }

	// RTVハンドルを取得する関数
	D3D12_CPU_DESCRIPTOR_HANDLE(&GetRtvHandles())[2] {return rtvHandles; }

	// 現在のバックバッファのインデックスを取得する関数
	UINT GetBackBufferIndex() { return backBufferIndex; }

	// バックバッファの数を取得する関数
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

	DissolveParams* GetDissolveParam() const { return dissolveParams_; }

	void SetTimeParams(float time) { timeParams_->time = time; }
private:
	// 関数
	// シングルトン用
	DirectXCommon() = default;
	~DirectXCommon() = default;
	DirectXCommon(const DirectXCommon&) = delete;
	DirectXCommon& operator=(const DirectXCommon&) = delete;

	
	
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
	const Vector4 kRenderTargetClearValue = { 0.1f,0.25f,0.5f,1.0f };

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

	Microsoft::WRL::ComPtr<ID3D12Resource> depthResource = nullptr;

	DepthMaterial* depthData = nullptr;

	// クラスメンバに追加
	D3D12_RESOURCE_STATES depthBufferState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	// mask用のResourceとアップロードリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> maskResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> maskUploadResource_;

	Microsoft::WRL::ComPtr<ID3D12Resource> dissolveParamBuffer_;
	DissolveParams* dissolveParams_ = nullptr;

	// ノイズを時間経過で変えるための変数
	Microsoft::WRL::ComPtr<ID3D12Resource> timeParamBuffer_;
	TimeParams* timeParams_ = nullptr;
};


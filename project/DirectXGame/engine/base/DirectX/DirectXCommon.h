#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "WinApp.h"
#include <array>
#include <dxcapi.h>
#include <DirectXTex.h>
#include "string"
#include "chrono"
#include <Vector4.h>
#include <DepthMaterial.h>
#include <ResourceManager.h>

// DirectXCommon用の定数
namespace DirectXCommonConstants {
	// バックバッファ数
	constexpr uint32_t kBackBufferCount = 2;
	
	// 最大SRV数(最大テクスチャ枚数)
	constexpr uint32_t kMaxSRVCount = 512;
	
	// デフォルトクリアカラー
	constexpr float kDefaultClearColorR = 0.1f;
	constexpr float kDefaultClearColorG = 0.25f;
	constexpr float kDefaultClearColorB = 0.5f;
	constexpr float kDefaultClearColorA = 1.0f;
	
	// FPS固定用
	constexpr float kTargetFPS = 60.0f;
	constexpr float kFPSCheckMargin = 65.0f;
	
	// デフォルトのDissolveパラメータ
	constexpr float kDefaultDissolveThreshold = 0.5f;
	constexpr float kDefaultDissolveEdgeWidth = 0.03f;
	constexpr float kDefaultDissolveEdgeStrength = 1.0f;
	constexpr float kDefaultDissolveEdgeColorR = 1.0f;
	constexpr float kDefaultDissolveEdgeColorG = 0.4f;
	constexpr float kDefaultDissolveEdgeColorB = 0.3f;
	
	// デフォルトの深度クリア値
	constexpr float kDefaultDepthClearValue = 1.0f;
	
	// シェーダーパス
	constexpr const wchar_t* kFullScreenVertexShaderPath = L"Resources/shaders/FullScreen.VS.hlsl";
	constexpr const wchar_t* kFullScreenPixelShaderPath = L"Resources/shaders/FullScreen.PS.hlsl";
	constexpr const wchar_t* kVertexShaderProfile = L"vs_6_0";
	constexpr const wchar_t* kPixelShaderProfile = L"ps_6_0";
	
	// デスクリプタヒープサイズ
	constexpr uint32_t kRTVDescriptorCount = 2;
	constexpr uint32_t kDSVDescriptorCount = 1;
	constexpr uint32_t kCBVSRVUAVDescriptorCount = 100;
	constexpr uint32_t kSRVDescriptorCount = 2;
	
	// ルートパラメータ数
	constexpr uint32_t kRootParameterCount = 4;
	constexpr uint32_t kStaticSamplerCount = 2;
	constexpr uint32_t kInputElementCount = 3;
	
	// ノイズテクスチャパス
	constexpr const char* kNoiseTexturePath = "resources/noise0.png";
}

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
public:
	// シングルトンインスタンス取得
	static DirectXCommon* GetInstance();

	// 初期化
	void Initialize(WinApp* winApp);

	// 描画前処理
	void PreRenderScene();
	void PreDraw();
	void PostDraw();

	// シェーダーコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);

	// BufferResourceの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	// DirectX12のTextureResourceを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);

	// TextureResourceにデータを転送する
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages);
	
	// ディスクリプタハンドル取得
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetDSVGPUDescriptorHandle(uint32_t index);

	// CPUとGPUの同期を待つ
	void SyncCPUWithGPU();

	// RenderTextureを描画
	void DrawRenderTexture();

	// デプスバッファの状態遷移
	void TransitionDepthBufferToSRV();
	void TransitionDepthBufferToWrite();

	// RenderTextureの状態遷移
	void TransitionRenderTextureToShaderResource();
	void TransitionRenderTextureToRenderTarget();

	// デプスリソース作成（外部から呼ばれる）
	void CreateDepthResource(Camera* camera);
	
	// マスクSRVディスクリプタヒープ作成（外部から呼ばれる）
	void CreateMaskSRVDescriptorHeap();

	// ディスクリプタヒープ作成（外部から呼ばれる）
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		D3D12_DESCRIPTOR_HEAP_TYPE heapType, 
		uint32_t numDescriptors, 
		bool shaderVisible);

	// ゲッター
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const { return device_; }
	IDxcUtils* GetDxcUtils() const { return dxcUtils_; }
	IDxcCompiler3* GetDxcCompiler() const { return dxcCompiler_; }
	IDxcIncludeHandler* GetIncludeHandler() const { return includeHandler_; }
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetSRVDescriptorHeap() const { return srvDescriptorHeap_; }
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDSVDescriptorHeap() const { return dsvDescriptorHeap_; }
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetCBVSRVUAVDescriptorHeap() const { return cbvSrvUavDescriptorHeap_; }
	uint32_t GetDescriptorSizeSRV() const { return descriptorSizeSRV_; }
	uint32_t GetDescriptorSizeDSV() const { return descriptorSizeDSV_; }
	Microsoft::WRL::ComPtr<IDXGISwapChain4> GetSwapChain() const { return swapChain_; }
	HANDLE GetFenceEvent() const { return fenceEvent_; }
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }
	D3D12_CPU_DESCRIPTOR_HANDLE(&GetRtvHandles())[2] { return rtvHandles_; }
	uint32_t GetBackBufferIndex() const { return backBufferIndex_; }
	uint32_t GetBackBufferCount() const { return DirectXCommonConstants::kBackBufferCount; }
	DissolveParams* GetDissolveParam() const { return dissolveParams_; }

	// セッター
	void SetTimeParams(float time) { timeParams_->time = time; }

	// 静的ヘルパー関数
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, 
		uint32_t descriptorSize, 
		uint32_t index);
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, 
		uint32_t descriptorSize, 
		uint32_t index);

	// 後方互換性のための静的メンバ
	static const uint32_t kMaxSRVCount_;

private:
	// コンストラクタ・デストラクタ
	DirectXCommon() = default;
	~DirectXCommon() = default;
	DirectXCommon(const DirectXCommon&) = delete;
	DirectXCommon& operator=(const DirectXCommon&) = delete;

	// 初期化ヘルパー関数
	void DeviceInitialize();
	void CommandInitialize();
	void SwapChain();
	void CreateBuffer(int32_t width, int32_t height);
	void DescriptorHeap();
	void RenderTargetView();
	void DepthStencilViewInitialize();
	void FenceInitialize();
	void ViewPortInitialize();
	void ScissorRectInitialize();
	void CreateDXCCompiler();
	void CreateRenderTexture();
	void CreateDepthSRVDescriptorHeap();
	void CreateDissolveParamBuffer();
	void CreateCBVSRVUAVDescriptorHeap(ID3D12Device* device);
	
	// ルートシグネチャとPSO
	void CreateRootSignature();
	void CreatePSO();
	
	// リソース作成ヘルパー
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(
		Microsoft::WRL::ComPtr<ID3D12Device> device, 
		int32_t width, 
		int32_t height);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTextureResoruce(
		Microsoft::WRL::ComPtr<ID3D12Device> device, 
		uint32_t width, 
		uint32_t height, 
		DXGI_FORMAT format, 
		const Vector4& clearColor);
	
	// FPS固定
	void initializeFixFPS();
	void UpdateFixFPS();
	
	// バリア設定
	void ChengeBarrier();
	
	// テクスチャ読み込み
	static DirectX::ScratchImage LoadTexture(const std::string& filePath);

	// デバイス初期化ヘルパー
	void EnableDebugLayer();
	void InitializeDXGIFactory();
	void SelectAdapter();
	void CreateD3D12Device();
	void ConfigureInfoQueue();
	
	// ルートシグネチャ/PSO設定ヘルパー
	void SetupRootParameters(D3D12_ROOT_PARAMETER* rootParameters, D3D12_DESCRIPTOR_RANGE* descriptorRange);
	void SetupStaticSamplers(D3D12_ROOT_SIGNATURE_DESC& rootSignatureDesc);
	void SetupInputLayout();
	void SetupBlendState();
	void SetupRasterizerState();
	void SetupDepthStencilState();
	void CompileShaders();

	// メンバ変数
	WinApp* winApp_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> selectedAdapter_;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};
	
	// デスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> cbvSrvUavDescriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> renderTextureRtvHeap_;
	
	// デスクリプタサイズ
	uint32_t descriptorSizeRTV_;
	uint32_t descriptorSizeSRV_;
	uint32_t descriptorSizeDSV_;
	
	// コマンド関連
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	
	// フェンス
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	uint64_t fenceValue_ = 0;
	HANDLE fenceEvent_ = nullptr;
	
	// リソース
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, DirectXCommonConstants::kBackBufferCount> swapChainResources_;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTexture_;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> maskResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> maskUploadResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> dissolveParamBuffer_;
	Microsoft::WRL::ComPtr<ID3D12Resource> timeParamBuffer_;
	
	// ビュー・パイプライン関連
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[DirectXCommonConstants::kBackBufferCount];
	D3D12_VIEWPORT viewport_{};
	D3D12_RECT scissorRect_{};
	D3D12_RESOURCE_BARRIER barrier_{};
	D3D12_SHADER_RESOURCE_VIEW_DESC renderTextureSrvDesc_{};
	
	// ルートシグネチャとパイプライン
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_{};
	D3D12_INPUT_ELEMENT_DESC inputElementDescs_[DirectXCommonConstants::kInputElementCount] = {};
	D3D12_RENDER_TARGET_BLEND_DESC blendDesc_{};
	D3D12_RASTERIZER_DESC rasterizerDesc_{};
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};
	
	// シェーダー
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;
	IDxcUtils* dxcUtils_ = nullptr;
	IDxcCompiler3* dxcCompiler_ = nullptr;
	IDxcIncludeHandler* includeHandler_ = nullptr;
	
	// 定数バッファデータ
	DissolveParams* dissolveParams_ = nullptr;
	TimeParams* timeParams_ = nullptr;
	DepthMaterial* depthData_ = nullptr;
	
	// 状態管理
	uint32_t backBufferIndex_ = 0;
	D3D12_RESOURCE_STATES renderTextureState_ = D3D12_RESOURCE_STATE_RENDER_TARGET;
	D3D12_RESOURCE_STATES depthBufferState_ = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	
	// クリアカラー
	const Vector4 kRenderTargetClearValue_ = { 
		DirectXCommonConstants::kDefaultClearColorR,
		DirectXCommonConstants::kDefaultClearColorG,
		DirectXCommonConstants::kDefaultClearColorB,
		DirectXCommonConstants::kDefaultClearColorA
	};
	
	// FPS固定用
	std::chrono::steady_clock::time_point reference_;
};


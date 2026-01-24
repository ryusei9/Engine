#pragma once
#include "Camera.h"
#include <memory>
#include <wrl.h>
#include <d3d12.h>
#include <dxcapi.h>

class SrvManager;
class DirectXCommon;

// Object3dCommon用の定数
namespace Object3dCommonConstants {
	// 入力レイアウトの要素数
	constexpr uint32_t kInputElementCount = 3;
	
	// ルートパラメータの数
	constexpr uint32_t kRootParameterCount = 8;
	
	// 静的サンプラの数
	constexpr uint32_t kStaticSamplerCount = 1;
	
	// DescriptorRangeの数
	constexpr uint32_t kDescriptorRangeCount = 2;
	
	// シェーダーパス
	constexpr const wchar_t* kVertexShaderPath = L"Resources/shaders/Object3d.VS.hlsl";
	constexpr const wchar_t* kPixelShaderPath = L"Resources/shaders/Object3d.PS.hlsl";
	constexpr const wchar_t* kVertexShaderProfile = L"vs_6_0";
	constexpr const wchar_t* kPixelShaderProfile = L"ps_6_0";
	
	// ルートパラメータインデックス
	constexpr uint32_t kRootParameterIndexMaterial = 0;
	constexpr uint32_t kRootParameterIndexTransformation = 1;
	constexpr uint32_t kRootParameterIndexTexture = 2;
	constexpr uint32_t kRootParameterIndexDirectionalLight = 3;
	constexpr uint32_t kRootParameterIndexCamera = 4;
	constexpr uint32_t kRootParameterIndexPointLight = 5;
	constexpr uint32_t kRootParameterIndexSpotLight = 6;
	constexpr uint32_t kRootParameterIndexEnvironmentMap = 7;
	
	// シェーダーレジスタ番号
	constexpr uint32_t kMaterialRegister = 0;
	constexpr uint32_t kTransformationRegister = 0;
	constexpr uint32_t kTextureRegister = 0;
	constexpr uint32_t kDirectionalLightRegister = 1;
	constexpr uint32_t kCameraRegister = 2;
	constexpr uint32_t kPointLightRegister = 3;
	constexpr uint32_t kSpotLightRegister = 4;
	constexpr uint32_t kEnvironmentMapRegister = 1;
	constexpr uint32_t kSamplerRegister = 0;
}

/// <summary>
/// 3Dオブジェクト共通部
/// </summary>
class Object3dCommon
{
public:
	// シングルトンインスタンスの取得
	static std::shared_ptr<Object3dCommon> GetInstance();

	// コンストラクタ・デストラクタ
	Object3dCommon() = default;
	~Object3dCommon() = default;
	
	// コピー禁止
	Object3dCommon(const Object3dCommon&) = delete;
	Object3dCommon& operator=(const Object3dCommon&) = delete;

	// 初期化
	void Initialize(SrvManager* srvManager);

	// 共通描画設定
	void DrawSettings();

	// セッター
	void SetDefaultCamera(Camera* camera) { defaultCamera_ = camera; }

	// ゲッター
	DirectXCommon* GetDxCommon() const { return dxCommon_; }
	Camera* GetDefaultCamera() const { return defaultCamera_; }

private:
	// ルートシグネチャの初期化
	void RootSignatureInitialize();

	// グラフィックスパイプラインの初期化
	void GraphicsPipelineInitialize();

	// 入力レイアウトの設定
	void SetupInputLayout();

	// ブレンドステートの設定
	void SetupBlendState();

	// ラスタライザステートの設定
	void SetupRasterizerState();

	// デプスステンシルステートの設定
	void SetupDepthStencilState();

	// シェーダーのコンパイル
	void CompileShaders();

	// ルートパラメータの設定
	void SetupRootParameters(D3D12_ROOT_PARAMETER* rootParameters, D3D12_DESCRIPTOR_RANGE* descriptorRanges);

	// 静的サンプラの設定
	void SetupStaticSamplers(D3D12_STATIC_SAMPLER_DESC* staticSamplers);

	// シングルトンインスタンス
	static std::shared_ptr<Object3dCommon> sInstance_;
	
	// DirectXCommon
	DirectXCommon* dxCommon_ = nullptr;

	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	// 入力レイアウト
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_{};
	D3D12_INPUT_ELEMENT_DESC inputElementDescs_[Object3dCommonConstants::kInputElementCount] = {};

	// ブレンドステート
	D3D12_RENDER_TARGET_BLEND_DESC blendDesc_{};

	// ラスタライザステート
	D3D12_RASTERIZER_DESC rasterizerDesc_{};

	// シェーダー
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

	// デプスステンシルステート
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};

	// グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_ = nullptr;

	// デフォルトカメラ
	Camera* defaultCamera_ = nullptr;

	// SrvManager
	SrvManager* srvManager_ = nullptr;
};


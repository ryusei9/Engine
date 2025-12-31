#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"

/// SpriteCommon調整用定数（マジックナンバー排除）
namespace SpriteCommonDefaults {
	// ルートパラメータ数（CBV x2, DescriptorTable x1）
	inline constexpr uint32_t kRootParamCount = 3;

	// 静的サンプラ数
	inline constexpr uint32_t kStaticSamplerCount = 1;

	// ディスクリプタレンジ数（テクスチャSRV）
	inline constexpr uint32_t kDescRangeCount = 1;

	// SRV ベースシェーダレジスタ
	inline constexpr uint32_t kSrvBaseRegister = 0;

	// CBV シェーダレジスタ（マテリアル・WVP共に b0）
	inline constexpr uint32_t kCbvRegister = 0;

	// 入力要素数（POSITION, TEXCOORD）
	inline constexpr uint32_t kInputElementCount = 2;

	// レンダーターゲット数
	inline constexpr uint32_t kRenderTargetCount = 1;

	// サンプル数
	inline constexpr uint32_t kSampleCount = 1;
}

/// <summary>
/// スプライト共通部
/// </summary>
class SpriteCommon
{
public:
	/*------メンバ関数------*/
	// シングルトンインスタンスの取得
	static std::shared_ptr<SpriteCommon> GetInstance();
	SpriteCommon() = default;
	~SpriteCommon() = default;
	SpriteCommon(SpriteCommon&) = delete;
	SpriteCommon& operator=(SpriteCommon&) = delete;

	// 初期化
	void Initialize(SrvManager* srvManager);

	// 共通描画設定
	void DrawSettings();

	/*------ゲッター------*/
	DirectXCommon* GetDxCommon() const { return dxCommon_; }

private:
	// シングルトンインスタンス
	static std::shared_ptr<SpriteCommon> sInstance_;
	
	// ルートシグネチャの初期化
	void RootSignatureInitialize();

	// グラフィックスパイプラインの初期化
	void GraphicsPipelineInitialize();

private:	// メンバ変数
	DirectXCommon* dxCommon_;

	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_{};

	D3D12_INPUT_ELEMENT_DESC inputElementDescs_[SpriteCommonDefaults::kInputElementCount] = {};

	/// BlendStateの設定
	D3D12_BLEND_DESC blendDesc_{};

	/// RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc_{};

	// shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;

	/// PixelShader
	// shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};

	// グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

	SrvManager* srvManager_ = nullptr;
};


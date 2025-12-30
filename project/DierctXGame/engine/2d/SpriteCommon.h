#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"

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

	D3D12_INPUT_ELEMENT_DESC inputElementDescs_[2] = {};

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


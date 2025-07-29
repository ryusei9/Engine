#pragma once
#include "DirectXCommon.h"

// スプライト共通部
class SpriteCommon
{
public:
	// シングルトンインスタンスの取得
	static std::shared_ptr<SpriteCommon> GetInstance();
	SpriteCommon() = default;
	~SpriteCommon() = default;
	SpriteCommon(SpriteCommon&) = delete;
	SpriteCommon& operator=(SpriteCommon&) = delete;

	// 初期化
	void Initialize();

	// 共通描画設定
	void DrawSettings();
	/// <summary>
	/// ゲッター
	/// </summary>
	DirectXCommon* GetDxCommon() const { return dxCommon_; }
private:	// メンバ関数
	// シングルトンインスタンス
	static std::shared_ptr<SpriteCommon> instance;
	
	// ルートシグネチャの初期化
	void RootSignatureInitialize();

	// グラフィックスパイプラインの初期化
	void GraphicsPipelineInitialize();

private:	// メンバ変数
	DirectXCommon* dxCommon_;

	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	/// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};

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
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState;
};


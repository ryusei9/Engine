#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <memory>
#include <cstdint>
#include <wrl/client.h>
#include <d3d12.h>
#include <dxcapi.h>

// 前方宣言
class DirectXCommon;
class SrvManager;

/// <summary>
/// SpriteCommon調整用定数（マジックナンバー排除）
/// </summary>
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

	// 入力要素数（POSITION, TEXCOORD, NORMAL）
	inline constexpr uint32_t kInputElementCount = 2;

	// レンダーターゲット数
	inline constexpr uint32_t kRenderTargetCount = 1;

	// サンプル数
	inline constexpr uint32_t kSampleCount = 1;

	// ルートパラメータインデックス
	inline constexpr uint32_t kRootParamIndexMaterial = 0;
	inline constexpr uint32_t kRootParamIndexWVP = 1;
	inline constexpr uint32_t kRootParamIndexTexture = 2;
}

/// <summary>
/// スプライト共通部（シングルトン）
/// </summary>
class SpriteCommon
{
public:
	/*------メンバ関数------*/

	// シングルトンインスタンスの取得
	static std::shared_ptr<SpriteCommon> GetInstance();

	// コンストラクタ・デストラクタ
	SpriteCommon() = default;
	~SpriteCommon() = default;

	// コピー・ムーブ禁止
	SpriteCommon(const SpriteCommon&) = delete;
	SpriteCommon& operator=(const SpriteCommon&) = delete;
	SpriteCommon(SpriteCommon&&) = delete;
	SpriteCommon& operator=(SpriteCommon&&) = delete;

	// 初期化
	void Initialize(SrvManager* srvManager);

	// 共通描画設定
	void DrawSettings();

	/*------ゲッター------*/

	// DirectXCommon の取得
	DirectXCommon* GetDxCommon() const { return dxCommon_; }

private:
	/*------プライベートメンバ関数------*/

	// ルートシグネチャの初期化
	void RootSignatureInitialize();

	// グラフィックスパイプラインの初期化
	void GraphicsPipelineInitialize();

	/*------メンバ変数------*/

	// シングルトンインスタンス
	static std::shared_ptr<SpriteCommon> sInstance_;

	// DirectX共通部
	DirectXCommon* dxCommon_ = nullptr;

	// SRVマネージャ
	SrvManager* srvManager_ = nullptr;

	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

	// 入力レイアウト記述
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_{};

	// 入力要素記述配列
	D3D12_INPUT_ELEMENT_DESC inputElementDescs_[SpriteCommonDefaults::kInputElementCount]{};

	// ブレンド設定
	D3D12_BLEND_DESC blendDesc_{};

	// ラスタライザ設定
	D3D12_RASTERIZER_DESC rasterizerDesc_{};

	// 頂点シェーダバイナリ
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;

	// ピクセルシェーダバイナリ
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

	// デプスステンシル設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};

	// グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
};


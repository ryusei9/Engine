#pragma once
#include "PostEffectBase.h"

/// <summary>
/// Noiseポストエフェクト
/// </summary>
class NoisePostEffect : public PostEffectBase
{
public:
	// 初期化
    void Initialize(DirectXCommon* dxCommon) override;

	// ルートシグネチャの作成
	void CreateRootSignature() override;

	// パイプラインステートオブジェクトの作成
	void CreatePipelineStateObject() override;

	// 描画前処理
    void PreRender() override;

	// 描画
    void Draw() override;

	// 描画後処理
    void PostRender() override;

	// エフェクト名の取得
    const char* GetName() const override { return "Noise"; }

	// OutputSRVの取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetOutputSRV() const override;

	// 時間パラメータの設定
    void SetTimeParams(float time) override { timeParams_->time = time; }

private:
    DirectXCommon* dxCommon_ = nullptr;

    // ルートシグネチャ
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    // パイプラインステート
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
    // シェーダーバイナリ
    Microsoft::WRL::ComPtr<IDxcBlob> vsBlob_;
    Microsoft::WRL::ComPtr<IDxcBlob> psBlob_;
    // SRV用ディスクリプタヒープ
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_{};

    /// RasterizerState
    D3D12_RASTERIZER_DESC rasterizerDesc_{};

    // DepthStencilStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};

    /// BlendStateの設定
    D3D12_RENDER_TARGET_BLEND_DESC blendDesc_{};

    // mask用のResourceとアップロードリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> maskResource_;
    Microsoft::WRL::ComPtr<ID3D12Resource> maskUploadResource_;

    Microsoft::WRL::ComPtr<ID3D12Resource> dissolveParamBuffer_;
    DissolveParams* dissolveParams_ = nullptr;

    // ノイズを時間経過で変えるための変数
    Microsoft::WRL::ComPtr<ID3D12Resource> timeParamBuffer_;
    TimeParams* timeParams_ = nullptr;
};


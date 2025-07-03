#pragma once
#include "PostEffectBase.h"
class NoisePostEffect : public PostEffectBase
{
public:
    void Initialize(DirectXCommon* dxCommon) override;

	void CreateRootSignature() override;
	void CreatePipelineStateObject() override;
    void PreRender() override;
    void Draw() override;
    void PostRender() override;
    const char* GetName() const override { return "Noise"; }

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

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};

    /// RasterizerState
    D3D12_RASTERIZER_DESC rasterizerDesc{};


    // DepthStencilStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

    /// BlendStateの設定
    D3D12_RENDER_TARGET_BLEND_DESC blendDesc{};

    // mask用のResourceとアップロードリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> maskResource_;
    Microsoft::WRL::ComPtr<ID3D12Resource> maskUploadResource_;

    Microsoft::WRL::ComPtr<ID3D12Resource> dissolveParamBuffer_;
    DissolveParams* dissolveParams_ = nullptr;

    // ノイズを時間経過で変えるための変数
    Microsoft::WRL::ComPtr<ID3D12Resource> timeParamBuffer_;
    TimeParams* timeParams_ = nullptr;
};


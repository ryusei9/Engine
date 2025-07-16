#pragma once
#include "PostEffectBase.h"

class GrayscalePostEffect : public PostEffectBase {
public:
    void Initialize(DirectXCommon* dxCommon) override;
    void PreRender() override;
    void Draw() override;
    void PostRender() override;
    void SetTimeParams(float time) override {}
    const char* GetName() const override { return "Gray"; }

    D3D12_GPU_DESCRIPTOR_HANDLE GetOutputSRV() const override;


private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
    Microsoft::WRL::ComPtr<IDxcBlob> vsBlob_;
    Microsoft::WRL::ComPtr<IDxcBlob> psBlob_;
    DirectXCommon* dxCommon_ = nullptr;

    void CreateRootSignature();
    void CreatePipelineStateObject();
};
#include "GrayscalePostEffect.h"
#include "SrvManager.h"
void GrayscalePostEffect::Initialize(DirectXCommon* dxCommon) {
    dxCommon_ = dxCommon;
    PostEffectBase::Initialize(dxCommon);
    CreateRootSignature();
    CreatePipelineStateObject();
    CreateRenderTexture(WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTargetClearValue);
}

void GrayscalePostEffect::CreateRootSignature() {
    D3D12_ROOT_PARAMETER rootParams[1]{};
    D3D12_DESCRIPTOR_RANGE range{};
    range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range.NumDescriptors = 1;
    range.BaseShaderRegister = 0;
    range.RegisterSpace = 0;
    range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[0].DescriptorTable.pDescriptorRanges = &range;
    rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.ShaderRegister = 0;
    samplerDesc.RegisterSpace = 0;
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
    rootSigDesc.NumParameters = _countof(rootParams);
    rootSigDesc.pParameters = rootParams;
    rootSigDesc.NumStaticSamplers = 1;
    rootSigDesc.pStaticSamplers = &samplerDesc;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    Microsoft::WRL::ComPtr<ID3DBlob> sigBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errorBlob);
    assert(SUCCEEDED(hr));
    hr = dxCommon_->GetDevice()->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));
}

void GrayscalePostEffect::CreatePipelineStateObject() {
    HRESULT hr;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    vsBlob_ = dxCommon_->CompileShader(L"Resources/shaders/FullScreen.VS.hlsl", L"vs_6_0");
    psBlob_ = dxCommon_->CompileShader(L"Resources/shaders/Grayscale.PS.hlsl", L"ps_6_0");
    psoDesc.pRootSignature = rootSignature_.Get();
    psoDesc.VS = { vsBlob_->GetBufferPointer(), vsBlob_->GetBufferSize() };
    psoDesc.PS = { psBlob_->GetBufferPointer(), psBlob_->GetBufferSize() };
    psoDesc.BlendState.RenderTarget[0].BlendEnable = false;
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.DepthStencilState.DepthEnable = false;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.InputLayout = { nullptr, 0 };
    hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_));
    assert(SUCCEEDED(hr));
}

void GrayscalePostEffect::PreRender() {
    dxCommon_->TransitionDepthBufferToWrite();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxCommon_->GetDSVCPUDescriptorHandle(0);
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
    float clearColor[4] = { 0.1f,0.25f,0.5f,1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);
}

void GrayscalePostEffect::Draw() {
    ID3D12DescriptorHeap* heaps[] = { dxCommon_->GetSRVDescriptorHeap().Get() };
    commandList->SetDescriptorHeaps(_countof(heaps), heaps);
    commandList->SetPipelineState(pipelineState_.Get());
    commandList->SetGraphicsRootSignature(rootSignature_.Get());
    commandList->SetGraphicsRootDescriptorTable(0, SrvManager::GetInstance()->GetGPUDescriptorHandle(1));
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);
}

void GrayscalePostEffect::PostRender() {
    dxCommon_->TransitionRenderTextureToShaderResource();
}

D3D12_GPU_DESCRIPTOR_HANDLE GrayscalePostEffect::GetOutputSRV() const
{
   return SrvManager::GetInstance()->GetGPUDescriptorHandle(srvIndex_);
}

#include "NoisePostEffect.h"

void NoisePostEffect::Initialize(DirectXCommon* dxCommon)
{
	// 引数を代入
	dxCommon_ = dxCommon;
	commandList = dxCommon_->GetCommandList();
    CreateRootSignature();
}

void NoisePostEffect::CreateRootSignature()
{
    // ルートシグネチャの作成（例：SRVとCBVのみ）
    D3D12_ROOT_PARAMETER rootParams[2]{};
    // SRV (t0)
    rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    D3D12_DESCRIPTOR_RANGE range{};
    range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range.NumDescriptors = 1;
    range.BaseShaderRegister = 0;
    range.RegisterSpace = 0;
    range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[0].DescriptorTable.pDescriptorRanges = &range;
    // CBV (b0)
    rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[1].Descriptor.ShaderRegister = 0;
    rootParams[1].Descriptor.RegisterSpace = 0;
    rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
    rootSigDesc.NumParameters = _countof(rootParams);
    rootSigDesc.pParameters = rootParams;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    Microsoft::WRL::ComPtr<ID3DBlob> sigBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errorBlob);
    assert(SUCCEEDED(hr));
    hr = dxCommon_->GetDevice()->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));

    /// InputLayout
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[2].SemanticName = "NORMAL";
    inputElementDescs[2].SemanticIndex = 0;
    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;


    inputLayoutDesc.pInputElementDescs = nullptr;
    inputLayoutDesc.NumElements = 0;

    /// BlendStateの設定
    // ブレンドするかしないか
    blendDesc.BlendEnable = false;
    // すべての色要素を書き込む
    blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    /// RasterizerState
    // 裏面(時計回り)を表示しない
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    // 三角形の中を塗りつぶす
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.FrontCounterClockwise = FALSE;	 // 時計回りの面を表面とする（カリング方向の設定）

    /// VertexShader
    // shaderをコンパイルする
    vsBlob_ = dxCommon_->CompileShader(L"Resources/shaders/FullScreen.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);

    /// PixelShader
    // shaderをコンパイルする
    psBlob_ = dxCommon_->CompileShader(L"Resources/shaders/Noise.PS.hlsl", L"ps_6_0");
    assert(pixelShaderBlob != nullptr);

    // DepthStencilStateの設定
    // Depthの機能を有効化する
    depthStencilDesc.DepthEnable = false;
    // 書き込みします
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    // 比較関数はLessEqual。つまり、近ければ描画される
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}

void NoisePostEffect::CreatePipelineStateObject()
{
    HRESULT hr;
    // PSOの作成
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = rootSignature_.Get();
    psoDesc.VS = { vsBlob_->GetBufferPointer(), vsBlob_->GetBufferSize() };
    psoDesc.PS = { psBlob_->GetBufferPointer(), psBlob_->GetBufferSize() };
    psoDesc.BlendState.RenderTarget[0] = blendDesc;
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.InputLayout = { nullptr, 0 }; // フルスクリーンクアッドは頂点レイアウト不要
    hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_));
    assert(SUCCEEDED(hr));
}

void NoisePostEffect::PreRender()
{
    // RenderTextureに描画するためのバリアやRTVセット
	TransitionRenderTextureToRenderTarget();
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dxCommon_->GetRtvHandles()[0];
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    // クリア
    float clearColor[4] = { 0.1f,0.25f,0.5f,1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
}

void NoisePostEffect::Draw()
{
    // パイプラインステートオブジェクトの設定
    commandList->SetPipelineState(pipelineState_.Get());
    // ルートシグネチャの設定
    commandList->SetGraphicsRootSignature(rootSignature_.Get());
    // SRVとCBVの設定
    commandList->SetGraphicsRootDescriptorTable(0, dxCommon_->GetSRVGPUDescriptorHandle(0));
    commandList->SetGraphicsRootConstantBufferView(1, timeParamBuffer_->GetGPUVirtualAddress());
    // プリミティブトポロジの設定
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // フルスクリーン四角形を描画
	commandList->DrawInstanced(3, 1, 0, 0);
}

void NoisePostEffect::PostRender()
{
    // バリアをSRV用に戻す
    dxCommon_->TransitionRenderTextureToShaderResource();
}

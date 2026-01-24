#include "NoisePostEffect.h"

//
// NoisePostEffect
// - フルスクリーンポストエフェクトを用いて画面にノイズ（時間変化する効果）を重畳するパス。
// - DirectX12 のルートシグネチャ／PSO を構築し、内部で作成したレンダーターゲットに対して
//   フルスクリーン三角形を描画してピクセルシェーダ（Noise.PS.hlsl）を実行することで、
//   グラフィックスパイプライン上でノイズを生成する。
// - 主な処理フロー：
//     Initialize()         : 必要なリソース（タイムパラメータCB、ルートシグネチャ、PSO、レンダーターゲット）を作成
//     PreRender()          : レンダーターゲットへの切替／クリア／ビューポート設定
//     Draw()               : 入力 SRV／CBV をバインドしてフルスクリーン三角形を描画
//     PostRender()         : 出力レンダーテクスチャを SRV として使える状態に遷移
//     GetOutputSRV() const : 本パスの出力 SRV ハンドルを取得（他パスで参照するため）
// - 注意点：
//   * 本クラスは PostEffectBase を継承し、CreateRenderTexture 等のユーティリティを利用している。
//   * 時間パラメータは Initialize 内で 0 初期化されるが、毎フレームの増分は呼び出し側で更新する必要がある。
//     （例: timeParams_->time を毎フレーム加算してから Draw を呼ぶ）
//   * SRV の入力ハンドルは dxCommon_->GetSRVGPUDescriptorHandle(1) など固定インデックスを参照しているため、
//     実行環境にあわせて入力 SRV の管理（インデックス割当）を整合させること。
//   * 深度テストは無効化されており、フルスクリーン描画専用の設定になっている。
//

void NoisePostEffect::Initialize(DirectXCommon* dxCommon)
{
	// 引数を保存し、ベースクラス初期化を行ったうえで本パス固有のリソースを構築する。
	// - timeParamBuffer_ : 時間等のパラメータを格納する CBV（CPU マップ可能）
	//   -> 呼び出し側が毎フレーム timeParams_->time を更新してノイズの時間変化を制御する
	// - ルートシグネチャ / PSO を生成
	// - 出力用のレンダーターゲクスチャを作成（幅/高は画面サイズ）
	dxCommon_ = dxCommon;
	PostEffectBase::Initialize(dxCommon);
    timeParamBuffer_ = dxCommon_->CreateBufferResource(sizeof(TimeParams));
    timeParamBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&timeParams_));
    timeParams_->time = 0.0f;
    CreateRootSignature();
	CreatePipelineStateObject();
	CreateRenderTexture(WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTargetClearValue_);
}

void NoisePostEffect::CreateRootSignature()
{
    // ルートシグネチャの構築
    // - rootParams[0] : SRV テーブル (t0) を 1 個（入力テクスチャや前段レンダー出力を参照）
    // - rootParams[1] : CBV (b0) を 1 個（TimeParams を格納）
    // - 静的サンプラを 1 個登録（ピクセルシェーダで利用）
    //
    // 補足:
    // - Shader 側のレジスタ割当 (t0, b0, s0) と合わせること。
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

    // サンプラー設定（線形、ラップ）
    D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.ShaderRegister = 0; // s0
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

    /// InputLayout
    // フルスクリーンクアッド（フルスクリーン三角形）を使用するため入力レイアウトは不要としている
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

    inputLayoutDesc_.pInputElementDescs = nullptr;
    inputLayoutDesc_.NumElements = 0;

    /// BlendStateの設定（ブレンド無効）
    blendDesc_.BlendEnable = false;
    blendDesc_.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    /// RasterizerState（カリング無し・塗りつぶし）
    rasterizerDesc_.CullMode = D3D12_CULL_MODE_NONE;
    rasterizerDesc_.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc_.FrontCounterClockwise = FALSE;	 // 時計回りを表面とする

    /// VertexShader / PixelShader のコンパイル
    vsBlob_ = dxCommon_->CompileShader(L"Resources/shaders/FullScreen.VS.hlsl", L"vs_6_0");
    assert(vsBlob_ != nullptr);

    psBlob_ = dxCommon_->CompileShader(L"Resources/shaders/Noise.PS.hlsl", L"ps_6_0");
    assert(psBlob_ != nullptr);

    // DepthStencilState の設定（深度テスト無効）
    depthStencilDesc_.DepthEnable = false;
    depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}

void NoisePostEffect::CreatePipelineStateObject()
{
    // PSO を構築する
    // - フルスクリーントライアングル用に入力レイアウトは空にしている
    HRESULT hr;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = rootSignature_.Get();
    psoDesc.VS = { vsBlob_->GetBufferPointer(), vsBlob_->GetBufferSize() };
    psoDesc.PS = { psBlob_->GetBufferPointer(), psBlob_->GetBufferSize() };
    psoDesc.BlendState.RenderTarget[0] = blendDesc_;
    psoDesc.RasterizerState = rasterizerDesc_;
    psoDesc.DepthStencilState = depthStencilDesc_;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.InputLayout = { nullptr, 0 }; // フルスクリーンクアッドは頂点レイアウト不要
    hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_));
    assert(SUCCEEDED(hr));
}

void NoisePostEffect::PreRender()
{
    // 描画前準備:
    // - 深度バッファを描画可能な状態に遷移
    // - 出力レンダーターゲットを OMSetRenderTargets でバインドしクリア
    // - ビューポート／シザーを設定
    dxCommon_->TransitionDepthBufferToWrite();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxCommon_->GetDSVCPUDescriptorHandle(0);
    commandList_->OMSetRenderTargets(1, &rtvHandle_, FALSE, &dsvHandle);
    float clearColor[4] = { 0.1f, 0.25f, 0.5f, 1.0f };
    commandList_->ClearRenderTargetView(rtvHandle_, clearColor, 0, nullptr);

    // ビューポート・シザーもセット（DirectXCommon の DrawRenderTexture と同様）
    commandList_->RSSetViewports(1, &viewport_);
    commandList_->RSSetScissorRects(1, &scissorRect_);
}

void NoisePostEffect::Draw()
{
    // 描画:
    // - SRV ヒープをセットし、PSO/RootSignature をバインド
    // - ルートに入力 SRV (t0) と時間パラメータ CBV (b0) を配置
    // - フルスクリーン三角形を DrawInstanced(3,1,0,0) で描画
    ID3D12DescriptorHeap* heaps[] = { dxCommon_->GetSRVDescriptorHeap().Get() };
    commandList_->SetDescriptorHeaps(_countof(heaps), heaps);

    commandList_->SetPipelineState(pipelineState_.Get());
    commandList_->SetGraphicsRootSignature(rootSignature_.Get());
    commandList_->SetGraphicsRootDescriptorTable(0, dxCommon_->GetSRVGPUDescriptorHandle(1));
    commandList_->SetGraphicsRootConstantBufferView(1, timeParamBuffer_->GetGPUVirtualAddress());
    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList_->DrawInstanced(3, 1, 0, 0);
}

void NoisePostEffect::PostRender()
{
    // 描画後処理:
    // - 出力レンダーターゲクスチャをシェーダリソース（SRV）として使用可能に遷移する
    dxCommon_->TransitionRenderTextureToShaderResource();
}

D3D12_GPU_DESCRIPTOR_HANDLE NoisePostEffect::GetOutputSRV() const
{
    // 本パスの出力 SRV ハンドルを取得するラッパ
    return dxCommon_->GetSRVGPUDescriptorHandle(srvIndex_);
}

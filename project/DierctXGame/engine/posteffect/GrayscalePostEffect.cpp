#include "GrayscalePostEffect.h"

//
// GrayscalePostEffect
// - フルスクリーンポストエフェクトで画面をグレースケールに変換するレンダーパス実装。
// - DirectX12 のルートシグネチャ / PSO を構築し、内部で作成したレンダーターゲット (RenderTexture) に
//   描画を行うことで、元のシーンをグレースケール化したシェーダ出力を生成する。
// - 使用手順（外部）:
//   1) Initialize(dxCommon) を呼んで初期化。
//   2) PreRender() でターゲットへの描画準備を行う。
//   3) Draw() を呼んでフルスクリーン三角形でピクセルシェーダを走らせる。
//   4) PostRender() でターゲットをシェーダリソースへ遷移する（他のパスで参照可能にする）。
//   5) GetOutputSRV() で出力 SRV を取得し、他のパス（UI 表示や別ポストエフェクト）へ渡す。
// - 注意点 / 実装メモ:
//   * このクラスは PostEffectBase を継承しており、CreateRenderTexture 等のユーティリティを利用する。
//   * サンプルでは SRV ヒープの 1 番（インデックス 1）を使用して入力テクスチャを参照する（dxCommon_->GetSRVGPUDescriptorHandle(1)）。
//     実運用では入力 SRV のインデックス管理に注意すること。
//   * PSO は深度テストを無効化しているため、フルスクリーン描画専用。
//   * RenderTexture のフォーマットは sRGB を用いている（出力の色空間を合わせること）。
//

void GrayscalePostEffect::Initialize(DirectXCommon* dxCommon) {
    // 初期化:
    // - DirectXCommon の参照を保持
    // - ベースクラスで共通初期化を実行
    // - ルートシグネチャ / PSO を作成
    // - 出力レンダーターゲット (RenderTexture) を作成（画面サイズ、フォーマット、クリア値指定）
    //
    // 入力:
    // - dxCommon: DirectX 初期化済みの共通ユーティリティ（デバイス・コマンド周りを提供）
    //
    // 副作用:
    // - GPU リソース（RenderTarget / SRV / RTV / DSV 等）を確保する
    dxCommon_ = dxCommon;
    PostEffectBase::Initialize(dxCommon);
    CreateRootSignature();
    CreatePipelineStateObject();
    CreateRenderTexture(WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTargetClearValue_);
}

void GrayscalePostEffect::CreateRootSignature() {
    // ルートシグネチャの作成:
    // - ピクセルシェーダで参照する SRV テーブル (t0) を 1 個持つだけのシンプルな構成
    // - サンプラを静的サンプラとして 1 個定義（線形フィルタ、ラップ）
    // - 入力アセンブラ入力レイアウトを使うためのフラグを有効化
    //
    // 注意:
    // - ここでは SRV テーブルのみをルートに持つ。必要に応じて定数バッファ等を追加する。
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
    // PSO (Pipeline State Object) の作成:
    // - フルスクリーン用頂点シェーダとグレースケール化ピクセルシェーダをコンパイルして設定
    // - ブレンド無効、ラスタライザはカリング無し、深度は使用しない設定
    // - 入力レイアウトはフルスクリーン三角形を使うため空にしている (IA は DrawInstanced で 3 頂点を描画)
    //
    // 副作用:
    // - pipelineState_ を生成する
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
    // 描画前準備:
    // - 深度バッファを描画可能な状態に遷移
    // - このパスの RTV を OMSetRenderTargets でバインド（DSV は深度バッファ）
    // - レンダーターゲットをクリアし、ビューポート / シザーをセット
    //
    // 副作用:
    // - コマンドリストに対するレンダーターゲット設定、クリアが行われる
    dxCommon_->TransitionDepthBufferToWrite();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxCommon_->GetDSVCPUDescriptorHandle(0);
    commandList_->OMSetRenderTargets(1, &rtvHandle_, FALSE, &dsvHandle);
    float clearColor[4] = { 0.1f, 0.25f, 0.5f, 1.0f };
    commandList_->ClearRenderTargetView(rtvHandle_, clearColor, 0, nullptr);
    commandList_->RSSetViewports(1, &viewport_);
    commandList_->RSSetScissorRects(1, &scissorRect_);
}

void GrayscalePostEffect::Draw() {
    // 実際の描画:
    // - SRV ヒープをコマンドリストにセット
    // - PSO / RootSignature をバインドし、ルートに入力 SRV を指定してフルスクリーン三角形を描画
    // - フルスクリーン三角形の DrawInstanced(3,1,0,0) によりピクセルシェーダがスクリーン各ピクセルを処理する
    //
    // 注意:
    // - ここでは入力 SRV を固定インデックス 1 のハンドルから取得している（dxCommon_->GetSRVGPUDescriptorHandle(1)）。
    //   実行環境によっては入力ソースの SRV 管理を呼び出し元で適切に行う必要がある。
    ID3D12DescriptorHeap* heaps[] = { dxCommon_->GetSRVDescriptorHeap().Get() };
    commandList_->SetDescriptorHeaps(_countof(heaps), heaps);
    commandList_->SetPipelineState(pipelineState_.Get());
    commandList_->SetGraphicsRootSignature(rootSignature_.Get());
    commandList_->SetGraphicsRootDescriptorTable(0, dxCommon_->GetSRVGPUDescriptorHandle(1));
    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList_->DrawInstanced(3, 1, 0, 0);
}

void GrayscalePostEffect::PostRender() {
    // 描画後処理:
    // - 出力レンダーテクスチャをシェーダリソースとして使用可能に遷移する（他のパスで SRV として読み込めるように）
    dxCommon_->TransitionRenderTextureToShaderResource();
}

D3D12_GPU_DESCRIPTOR_HANDLE GrayscalePostEffect::GetOutputSRV() const
{
    // 出力レンダーターゲットの SRV ハンドルを返す
    return dxCommon_->GetSRVGPUDescriptorHandle(srvIndex_);
}

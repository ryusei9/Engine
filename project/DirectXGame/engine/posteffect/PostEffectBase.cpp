#include "PostEffectBase.h"
#include "SrvManager.h"

//
// PostEffectBase
// - ポストプロセス共通のユーティリティ基底クラス。
// - 目的:
//   * フルスクリーンポストエフェクトパスで必要となる共通処理（ビューポート／シザーの初期化、
//     レンダーテクスチャの作成と状態遷移、コマンドリスト参照の保持など）を提供する。
// - 想定利用フロー:
//   1) 派生クラスで Initialize(dxCommon) を呼ぶ（基底でコマンドリストとビューポート等を初期化）
//   2) CreateRenderTexture(...) で出力用レンダーターゲットを確保
//   3) PreRender()/Draw()/PostRender() 相当の処理を派生クラスで実装して使用
//   4) GetOutputSRV() 等で出力を他パスに渡す
// - 実装上の注意点:
//   * 本クラスは DirectX リソースのライフサイクル（CreateCommittedResource など）を扱うため、呼び出し側で
//     dxCommon_（DirectXCommon）の初期化が済んでいることが前提です。
//   * CreateRenderTexture は Default ヒープ上にレンダーターゲットを作成し、RTV と SRV（dxCommon_ の SRV ヒープ）を生成します。
//   * リソース状態遷移は ResourceBarrier で明示的に行い、内部変数 renderTextureState_ を更新して二重遷移を避けます。
//   * スレッドセーフではありません。レンダースレッド（メインスレッド）で利用してください。
//


void PostEffectBase::Initialize(DirectXCommon* dxCommon) {
    // 初期化:
    // - DirectXCommon の参照を保持してコマンドリストを取得する。
    // - ビューポートとシザー矩形を画面サイズに合わせて初期化する。
    //
    // 前提:
    // - dxCommon は既に初期化済みであること（デバイスやスワップチェインが生成済み）。
    dxCommon_ = dxCommon;
    commandList_ = dxCommon_->GetCommandList();
    ViewPortInitialize();
    ScissorRectInitialize();
}

void PostEffectBase::ViewPortInitialize() {
    // ビューポートの初期化:
    // - 幅／高さは WinApp のクライアントサイズを使用する（デフォルトの画面サイズ）。
    // - MinDepth/MaxDepth は [0,1] に設定。
    // - TopLeft は (0,0)。
    viewport_.Width = static_cast<float>(WinApp::kClientWidth);
    viewport_.Height = static_cast<float>(WinApp::kClientHeight);
    viewport_.MinDepth = 0.0f;
    viewport_.MaxDepth = 1.0f;
    viewport_.TopLeftX = 0.0f;
    viewport_.TopLeftY = 0.0f;
}

void PostEffectBase::ScissorRectInitialize() {
    // シザー矩形の初期化:
    // - left/top を 0 に、right/bottom をクライアント幅・高さに合わせる。
    scissorRect_.left = 0;
    scissorRect_.right = WinApp::kClientWidth;
    scissorRect_.top = 0;
    scissorRect_.bottom = WinApp::kClientHeight;
}

void PostEffectBase::TransitionRenderTextureToRenderTarget() {
    // レンダーテクスチャを RTV (レンダーターゲット) 用に遷移するユーティリティ。
    // - 現在の状態が既に RENDER_TARGET であれば何もしない（冪等性を担保）。
    // - それ以外の場合は PIXEL_SHADER_RESOURCE -> RENDER_TARGET の遷移バリアを発行する。
    // - 実行後、内部状態 renderTextureState_ を更新する。
	if (renderTextureState_ == D3D12_RESOURCE_STATE_RENDER_TARGET) return;
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = renderTexture_.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	commandList_->ResourceBarrier(1, &barrier);

	// 状態を更新して以降の遷移呼び出しで無駄なバリア発行を防ぐ
	renderTextureState_ = D3D12_RESOURCE_STATE_RENDER_TARGET;
}

void PostEffectBase::TransitionRenderTextureToShaderResource() {
    // レンダーテクスチャをシェーダリソース（SRV）用に遷移するユーティリティ。
    // - 現在の状態が既に PIXEL_SHADER_RESOURCE であれば何もしない（冪等性を担保）。
    // - それ以外の場合は RENDER_TARGET -> PIXEL_SHADER_RESOURCE の遷移バリアを発行する。
    // - 実行後、内部状態 renderTextureState_ を更新する。
	if (renderTextureState_ == D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) return;
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = renderTexture_.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	commandList_->ResourceBarrier(1, &barrier);
	renderTextureState_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
}

void PostEffectBase::CreateRenderTexture(UINT width, UINT height, DXGI_FORMAT format, const Vector4& clearColor) {
    // 出力用レンダーテクスチャ（Render Target）を生成し、
    // - Default ヒープ上にテクスチャリソースを作成
    // - このリソース用の RTV ヒープを作成して RTV を生成
    // - dxCommon_ が管理する SRV ヒープの指定インデックスへ SRV を書き込む
    //
    // 引数:
    // - width/height : レンダーテクスチャのサイズ（通常はスワップチェインのサイズと同じ）
    // - format        : 出力フォーマット（例: DXGI_FORMAT_R8G8B8A8_UNORM_SRGB）
    // - clearColor    : RTV クリア時に使う色（RGBA）
    //
    // 注意:
    // - SRV はここでは dxCommon_->GetSRVCPUDescriptorHandle(1) へ作成する例になっている（インデックス管理に注意）。
    // - CreateCommittedResource の初期状態は RENDER_TARGET にしている（すぐに書き込み可能）。
    // - 複数フレーム用にフレーム分のリソースを用意する等は行っておらず、単一の出力テクスチャ想定。
    //
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Width = width;
    desc.Height = height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = format;
    clearValue.Color[0] = clearColor.x;
    clearValue.Color[1] = clearColor.y;
    clearValue.Color[2] = clearColor.z;
    clearValue.Color[3] = clearColor.w;

    HRESULT hr = dxCommon_->GetDevice()->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue,
        IID_PPV_ARGS(&renderTexture_));
    assert(SUCCEEDED(hr));

    // RTV ヒープ（CPU only）を作成して RTV を生成
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 1;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    hr = dxCommon_->GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap_));
    assert(SUCCEEDED(hr));
    rtvHandle_ = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
    dxCommon_->GetDevice()->CreateRenderTargetView(renderTexture_.Get(), nullptr, rtvHandle_);

    // SRV を dxCommon_ の SRV ヒープへ作成（ここでは例として index=1 を使用）
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;

    // 注意: dxCommon_->GetSRVCPUDescriptorHandle(1) のインデックス管理は呼び出し側で合わせること
    dxCommon_->GetDevice()->CreateShaderResourceView(renderTexture_.Get(), &srvDesc, dxCommon_->GetSRVCPUDescriptorHandle(1));

    // 初期のリソース状態を RENDER_TARGET として記録
    renderTextureState_ = D3D12_RESOURCE_STATE_RENDER_TARGET;
}

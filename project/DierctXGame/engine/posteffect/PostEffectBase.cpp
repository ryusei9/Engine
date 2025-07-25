#include "PostEffectBase.h"
#include "SrvManager.h"
void PostEffectBase::Initialize(DirectXCommon* dxCommon) {
    this->dxCommon = dxCommon;
    commandList = dxCommon->GetCommandList();
    ViewPortInitialize();
    ScissorRectInitialize();
}

void PostEffectBase::ViewPortInitialize() {
    viewport.Width = static_cast<float>(WinApp::kClientWidth);
    viewport.Height = static_cast<float>(WinApp::kClientHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
}

void PostEffectBase::ScissorRectInitialize() {
    scissorRect.left = 0;
    scissorRect.right = WinApp::kClientWidth;
    scissorRect.top = 0;
    scissorRect.bottom = WinApp::kClientHeight;
}

void PostEffectBase::TransitionRenderTextureToRenderTarget() {
	if (renderTextureState == D3D12_RESOURCE_STATE_RENDER_TARGET) return;
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = renderTexture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	commandList->ResourceBarrier(1, &barrier);

	// 🔧 状態を更新！
	renderTextureState = D3D12_RESOURCE_STATE_RENDER_TARGET;
}

void PostEffectBase::TransitionRenderTextureToShaderResource() {
	if (renderTextureState == D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) return;
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = renderTexture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	commandList->ResourceBarrier(1, &barrier);
	renderTextureState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
}

void PostEffectBase::CreateRenderTexture(UINT width, UINT height, DXGI_FORMAT format, const Vector4& clearColor)
{
    // RenderTextureリソース生成
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

    HRESULT hr = dxCommon->GetDevice()->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue,
        IID_PPV_ARGS(&renderTexture));
    assert(SUCCEEDED(hr));

    // RTVヒープ作成
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 1;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    hr = dxCommon->GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
    assert(SUCCEEDED(hr));
    rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    dxCommon->GetDevice()->CreateRenderTargetView(renderTexture.Get(), nullptr, rtvHandle);

    // SRVの作成（dxCommonのSRVヒープを使う。インデックスは用途に応じて指定）
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;

    // 例: 0番にSRVを作成
    dxCommon->GetDevice()->CreateShaderResourceView(renderTexture.Get(), &srvDesc, SrvManager::GetInstance()->GetCPUDescriptorHandle(1));

    renderTextureState = D3D12_RESOURCE_STATE_RENDER_TARGET;
}

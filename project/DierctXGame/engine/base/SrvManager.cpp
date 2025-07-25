#include "SrvManager.h"
#include <cassert>

const uint32_t SrvManager::kMaxSRVCount = 512;

SrvManager* SrvManager::GetInstance()
{
	static SrvManager instance;
	return &instance;
}

void SrvManager::Initialize()
{
	// 引数で受け取ってメンバ変数に記録する
	directXCommon_ = DirectXCommon::GetInstance();

	// デスクリプタヒープ
	descriptorHeap = directXCommon_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);

	// デスクリプタ1個分のサイズを取得して記録
	descriptorSize = directXCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// 空きインデックスリストを初期化
	for (uint32_t i = 0; i < kMaxSRVCount; ++i) {
		freeIndices.push(i);
	}
}

uint32_t SrvManager::Allocate()
{
	// 空きインデックスがあるかをチェック
	assert(!freeIndices.empty() && "No available SRV slots!");

	// 空きインデックスリストから番号を取得
	uint32_t index = freeIndices.front();
	freeIndices.pop();

	return index;

}

void SrvManager::Free(uint32_t srvIndex)
{
	// インデックスが範囲内であることを確認
	assert(srvIndex < kMaxSRVCount && "Invalid SRV index to free!");

	// 空きインデックスリストに戻す
	freeIndices.push(srvIndex);
}

void SrvManager::CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	// 2Dテクスチャ
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(MipLevels);

	directXCommon_->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, GetCPUDescriptorHandle(srvIndex));
}

void SrvManager::CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structuredByteStride)
{
	// SRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN; // 構造化バッファの場合、フォーマットは UNKNOWN
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	// StructuredBuffer の設定
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER; // バッファとして扱う
	srvDesc.Buffer.FirstElement = 0; // バッファの先頭から始める
	srvDesc.Buffer.NumElements = numElements; // バッファ内の要素数
	srvDesc.Buffer.StructureByteStride = structuredByteStride; // 1要素あたりのバイト数
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE; // 特にフラグなし

	// SRV を作成
	directXCommon_->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, GetCPUDescriptorHandle(srvIndex));
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>  SrvManager::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
{
	// ディスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = numDescriptors;
	heapDesc.Type = heapType;
	heapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	HRESULT hr = directXCommon_->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr) && "Failed to create descriptor heap!");
	return descriptorHeap;
}

void SrvManager::CreateDepthSRVDescriptorHeap()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC depthTextureSrvDesc = {};

	depthTextureSrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	depthTextureSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	depthTextureSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	depthTextureSrvDesc.Texture2D.MipLevels = 1;
	// 1番に「深度テクスチャ（gDepthTexture）」を登録
	directXCommon_->GetDevice()->CreateShaderResourceView(directXCommon_->GetDepthStencilResource().Get(), &depthTextureSrvDesc, SrvManager::GetInstance()->GetCPUDescriptorHandle(1));
}

void SrvManager::PreDraw()
{
	// 描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap.Get() };
	directXCommon_->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
}

void SrvManager::SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex)
{
	directXCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(RootParameterIndex, GetGPUDescriptorHandle(srvIndex));
}

D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

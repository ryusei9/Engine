#include "SrvManager.h"
#include <cassert>

//
// SrvManager
// - GPU 用の SRV (Shader Resource View) デスクリプタを管理するユーティリティ。
// - DescriptorHeap を内部で1つ保持し、SRV を配列インデックス (0..kMaxSRVCount-1) で割り当て/解放できる。
// - 主な機能:
//   * Initialize(): DescriptorHeap の確保と空きインデックスの初期化
//   * Allocate()/Free(): SRV スロットの割当と解放（FIFO の空きリスト）
//   * CreateSRVforTexture2D()/CreateSRVforStructuredBuffer(): 指定リソースに対する SRV を作成して descriptor heap に書き込む
//   * PreDraw()/SetGraphicsRootDescriptorTable()/GetCPU/GPUDescriptorHandle(): 描画時に使うハンドル取得とルートへの設定
//
// 注意点:
// - このクラスは DescriptorHeap を Shader-visible に作成するため、SRV を GPU 側で直接参照できる。
// - GetCPUDescriptorHandle/GetGPUDescriptorHandle はインデックスに基づいてヒープ上のハンドルを計算するため、
//  呼び出し元は Allocate() で取得した有効なインデックスを必ず使用すること。
// - 現実的なレンダラーではスレッド安全性やデスクリプタ更新の同期（フレームバッファごとのデスクリプタスロット分離）を考慮する必要があるが
//   本実装は単純化のため単一ヒープかつ単純な空き管理（queue）としている。
// - descriptorSize_ はデバイス依存（GetDescriptorHandleIncrementSize で取得）で、ハンドル計算時に使用する。
//

using namespace SrvManagerConstants;

const uint32_t SrvManager::kMaxSRVCount_ = kMaxSRVCount;

void SrvManager::Initialize()
{
	// DirectXCommon インスタンスを取得して保持
	directXCommon_ = DirectXCommon::GetInstance();

	// DescriptorHeap を作成する
	descriptorHeap_ = directXCommon_->CreateDescriptorHeap(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 
		kMaxSRVCount, 
		true);

	// デスクリプタのステップサイズを取得
	descriptorSize_ = directXCommon_->GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// 空きインデックスキューを初期化
	InitializeFreeIndices();
}

uint32_t SrvManager::Allocate()
{
	// 空きスロットの確認
	assert(HasFreeSlot() && "No available SRV slots!");

	// キューから1つ取り出して割り当て番号として返す
	uint32_t index = freeIndices_.front();
	freeIndices_.pop();

	return index;
}

void SrvManager::Free(uint32_t srvIndex)
{
	// インデックスの検証
	assert(ValidateIndex(srvIndex) && "Invalid SRV index to free!");

	// 再利用のためキューに戻す
	freeIndices_.push(srvIndex);
}

void SrvManager::CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels)
{
	// nullptrチェック
	assert(pResource != nullptr && "Resource pointer is null!");
	assert(ValidateIndex(srvIndex) && "Invalid SRV index!");

	// 2D テクスチャ用 SRV を構築
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = MipLevels;

	CreateSRVInternal(srvIndex, pResource, srvDesc);
}

void SrvManager::CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structuredByteStride)
{
	// nullptrチェック
	assert(pResource != nullptr && "Resource pointer is null!");
	assert(ValidateIndex(srvIndex) && "Invalid SRV index!");
	assert(numElements > 0 && "Number of elements must be greater than 0!");
	assert(structuredByteStride > 0 && "Structure byte stride must be greater than 0!");

	// 構造化バッファ用の SRV を構築
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = kDefaultFirstElement;
	srvDesc.Buffer.NumElements = numElements;
	srvDesc.Buffer.StructureByteStride = structuredByteStride;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	CreateSRVInternal(srvIndex, pResource, srvDesc);
}

void SrvManager::PreDraw()
{
	// 描画前に DescriptorHeap をコマンドリストに設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap_.Get() };
	directXCommon_->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
}

void SrvManager::SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex)
{
	// インデックスの検証
	assert(ValidateIndex(srvIndex) && "Invalid SRV index!");

	// 指定のルートパラメータに GPU 側のデスクリプタハンドルを設定
	directXCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(
		RootParameterIndex, 
		GetGPUDescriptorHandle(srvIndex));
}

D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index)
{
	// インデックスの検証
	assert(ValidateIndex(index) && "Invalid SRV index!");

	// DescriptorHeap の先頭ハンドルを基準にインデックス分オフセット
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize_ * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index)
{
	// インデックスの検証
	assert(ValidateIndex(index) && "Invalid SRV index!");

	// DescriptorHeap の先頭ハンドルを基準にインデックス分オフセット
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize_ * index);
	return handleGPU;
}

bool SrvManager::IsWithinMaxSRVCount(uint32_t srvIndex) const
{
	return srvIndex < kMaxSRVCount;
}

// ===== ヘルパー関数 =====

void SrvManager::CreateSRVInternal(uint32_t srvIndex, ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc)
{
	// SRV を指定スロットに書き込む共通処理
	directXCommon_->GetDevice()->CreateShaderResourceView(
		pResource, 
		&srvDesc, 
		GetCPUDescriptorHandle(srvIndex));
}

bool SrvManager::ValidateIndex(uint32_t srvIndex) const
{
	// インデックスが有効範囲内かチェック
	return srvIndex < kMaxSRVCount;
}

void SrvManager::InitializeFreeIndices()
{
	// 空きインデックスキューを初期化（0..kMaxSRVCount-1 を push）
	for (uint32_t i = 0; i < kMaxSRVCount; ++i) {
		freeIndices_.push(i);
	}
}

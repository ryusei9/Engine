#include "SrvManager.h"
#include <cassert>

const uint32_t SrvManager::kMaxSRVCount = 512;

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
// - descriptorSize はデバイス依存（GetDescriptorHandleIncrementSize で取得）で、ハンドル計算時に使用する。
//

void SrvManager::Initialize()
{
	// DirectXCommon インスタンスを取得して保持
	this->directXCommon = DirectXCommon::GetInstance();

	// DescriptorHeap を作成する:
	// - 種別: CBV/SRV/UAV
	// - エントリ数: kMaxSRVCount（定数）
	// - Shader-visible にしてシェーダ側から参照可能にする
	descriptorHeap = this->directXCommon->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);

	// デスクリプタのステップサイズを取得（CPU/GPU ハンドル計算で使用）
	descriptorSize = this->directXCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// 空きインデックスキューを初期化（0..kMaxSRVCount-1 を push）
	for (uint32_t i = 0; i < kMaxSRVCount; ++i) {
		freeIndices.push(i);
	}
}

uint32_t SrvManager::Allocate()
{
	// 空きインデックスが存在することを保証
	assert(!freeIndices.empty() && "No available SRV slots!");

	// キューから1つ取り出して割り当て番号として返す
	uint32_t index = freeIndices.front();
	freeIndices.pop();

	return index;
}

void SrvManager::Free(uint32_t srvIndex)
{
	// 解放するインデックスが範囲内であることをチェック
	assert(srvIndex < kMaxSRVCount && "Invalid SRV index to free!");

	// 再利用のためキューに戻す
	freeIndices.push(srvIndex);
}

void SrvManager::CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels)
{
	// 2D テクスチャ用 SRV を構築して DescriptorHeap の指定スロットに書き込む
	// パラメータ:
	//  - srvIndex: GetCPUDescriptorHandle/GetGPUDescriptorHandle で指定するインデックス
	//  - pResource: SRV を作る対象の ID3D12Resource*
	//  - Format: テクスチャのフォーマット
	//  - MipLevels: ミップレベル数
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(MipLevels);

	directXCommon->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, GetCPUDescriptorHandle(srvIndex));
}

void SrvManager::CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structuredByteStride)
{
	// 構造化バッファ (StructuredBuffer) 用の SRV を構築して指定スロットに書き込む
	// - structuredByteStride: 1要素あたりのバイト数
	// - numElements: 要素数
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN; // 構造化バッファはフォーマット指定なし
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = numElements;
	srvDesc.Buffer.StructureByteStride = structuredByteStride;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	directXCommon->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, GetCPUDescriptorHandle(srvIndex));
}

void SrvManager::PreDraw()
{
	// 描画前に DescriptorHeap をコマンドリストに設定する補助関数
	// - 各フレームの描画開始時に呼ぶことで、ルートに設定する GPU ハンドルが有効になる
	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap.Get() };
	directXCommon->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
}

void SrvManager::SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex)
{
	// 指定のルートパラメータに GPU 側のデスクリプタハンドルを設定するラッパ
	directXCommon->GetCommandList()->SetGraphicsRootDescriptorTable(RootParameterIndex, GetGPUDescriptorHandle(srvIndex));
}

D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index)
{
	// DescriptorHeap の先頭ハンドルを基準にインデックス分オフセットして CPU ハンドルを返す
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index)
{
	// DescriptorHeap の先頭ハンドルを基準にインデックス分オフセットして GPU ハンドルを返す
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

#pragma once
#include <wrl.h>
#include <queue>
#include "DirectXCommon.h"

// SrvManager用の定数
namespace SrvManagerConstants {
	// 最大SRV数(最大テクスチャ枚数)
	constexpr uint32_t kMaxSRVCount = 512;
	
	// デスクリプタの初期化値
	constexpr uint32_t kInitialUseIndex = 0;
	
	// デフォルトのMipLevels
	constexpr UINT kDefaultMipLevels = 1;
	
	// 構造化バッファの初期要素
	constexpr UINT kDefaultFirstElement = 0;
}

/// <summary>
/// SRV管理
/// </summary>
class SrvManager
{
public:
	// 初期化
	void Initialize();

	// SRV確保
	uint32_t Allocate();

	// SRV解放
	void Free(uint32_t srvIndex);

	// SRV生成(texture用)
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels);

	// SRV生成(Structured Buffer用)
	void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structuredByteStride);

	// 描画前処理
	void PreDraw();

	// グラフィックスルートデスクリプタテーブル設定
	void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);

	// SRVの指定番号のCPUデスクリプタハンドルを取得する
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);

	// SRVの指定番号のGPUデスクリプタハンドルを取得する
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	// ゲッター
	ID3D12DescriptorHeap* GetDescriptorHeap() const { return descriptorHeap_.Get(); }
	uint32_t GetDescriptorSize() const { return descriptorSize_; }
	uint32_t GetUseIndex() const { return useIndex_; }
	size_t GetFreeSlotCount() const { return freeIndices_.size(); }
	uint32_t GetMaxSRVCount() const { return SrvManagerConstants::kMaxSRVCount; }

	// 指定されたSRVインデックスが最大SRV数を超えているか確認する
	bool IsWithinMaxSRVCount(uint32_t srvIndex) const;
	
	// 空きスロットがあるかチェック
	bool HasFreeSlot() const { return !freeIndices_.empty(); }

	// 後方互換性のための静的メンバ
	static const uint32_t kMaxSRVCount_;

private:
	// SRV作成の共通処理
	void CreateSRVInternal(uint32_t srvIndex, ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc);
	
	// インデックスの検証
	bool ValidateIndex(uint32_t srvIndex) const;
	
	// 空きインデックスの初期化
	void InitializeFreeIndices();

	// DirectXCommon
	DirectXCommon* directXCommon_ = nullptr;

	// SRV用のデスクリプタサイズ
	uint32_t descriptorSize_ = 0;

	// SRV用デスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;

	// 次に使用するSRVインデックス
	uint32_t useIndex_ = SrvManagerConstants::kInitialUseIndex;

	// 空いているSRVインデックスリスト
	std::queue<uint32_t> freeIndices_;
};


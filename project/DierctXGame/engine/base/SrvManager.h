#pragma once
#include <wrl.h>
#include <queue>
#include "DirectXCommon.h"

// SRV管理
class SrvManager
{

public:
	// シングルトンインスタンス取得
	static SrvManager* GetInstance();
	
	// メンバ関数
	// 初期化
	void Initialize();

	uint32_t Allocate();

	void Free(uint32_t srvIndex);

	// SRV生成(texture用)
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels);

	// SRV生成(Structured Buffer用)
	void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structuredByteStride);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	void CreateDepthSRVDescriptorHeap();

	void PreDraw();

	void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);

	/// <summary>
	/// SRVの指定番号のCPUデスクリプタハンドルを取得する
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);

	/// <summary>
	/// SRVの指定番号のGPUデスクリプタハンドルを取得する
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	// ディスクリプタヒープの取得
	ID3D12DescriptorHeap* GetDescriptorHeap() { return descriptorHeap.Get(); }

	/// <summary>
/// 指定されたSRVインデックスが最大SRV数を超えているか確認する
/// </summary>
/// <param name="srvIndex">確認するSRVインデックス</param>
/// <returns>最大SRV数を超えていなければtrue、超えていればfalse</returns>
	bool IsWithinMaxSRVCount(uint32_t srvIndex) const
	{
		return srvIndex < kMaxSRVCount;
	}

	// 最大SRV数(最大テクスチャ枚数)
	static const uint32_t kMaxSRVCount;
private:
	SrvManager() = default;
	~SrvManager() = default;
	SrvManager(const SrvManager&) = delete;
	SrvManager& operator=(const SrvManager&) = delete;

	// メンバ変数
	DirectXCommon* directXCommon_ = nullptr;

	

	// SRV用のデスクリプタサイズ
	uint32_t descriptorSize;

	// SRV用デスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	// 次に使用するSRVインデックス
	uint32_t useIndex = 0;

	// 空いているSRVインデックスリスト
	std::queue<uint32_t> freeIndices;
};


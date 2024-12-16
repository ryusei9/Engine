#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include <SrvManager.h>
// ImGuiの管理
class ImGuiManager
{
public:
	/// 初期化
	void Initialize(WinApp* winApp_,DirectXCommon* dxCommon,SrvManager* srvManager);

private:
	WinApp* winApp_;

	DirectXCommon* dxCommon_;
	// SRV用デスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;
	
};


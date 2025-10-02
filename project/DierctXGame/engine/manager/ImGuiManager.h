#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"

// ImGuiの管理
class ImGuiManager
{
public:
	/// 初期化
	void Initialize(WinApp* winApp_);

	/// ImGui受付開始
	void Begin();

	/// ImGui受付終了
	void End();

	/// 描画
	void Draw();

	/// <summary>
	/// 終了
	/// </summary>
	void Finalize();
private:
	WinApp* winApp_;

	DirectXCommon* dxCommon_;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;
};


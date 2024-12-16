#include "ImGuiManager.h"
#include "ImGui/imgui.h"
#include <ImGui/imgui_impl_win32.h>
#include <ImGui/imgui_impl_dx12.h>
void ImGuiManager::Initialize(WinApp* winApp_, DirectXCommon* dxCommon,SrvManager* srvManager)
{
	dxCommon_ = dxCommon;
	uint32_t index = srvManager->Allocate();
	// ImGuiの初期化
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp_->GetHwnd());
    ImGui_ImplDX12_Init(dxCommon_->GetDevice().Get(),
		dxCommon_->GetBackBufferCount(),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		srvManager->GetDescriptorHeap(),
		srvManager->GetCPUDescriptorHandle(index),
		srvManager->GetGPUDescriptorHandle(index));
}

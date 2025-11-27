#include "ImGuiManager.h"
#ifdef USE_IMGUI
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#endif

//
// ImGuiManager
// - Dear ImGui を DirectX12 + Win32 環境で初期化・管理するユーティリティクラス。
// - 役割：ImGui コンテキストの生成／破棄、フレーム開始・終了処理、描画コマンドの発行、および
//         ImGui 用のシェーダから参照される SRV 用のデスクリプタヒープを用意する。
// - 設計上の注意点：
//   * ImGui の初期化は USE_IMGUI マクロで有効化される実装に依存している（ビルド時の切替）。
//   * DescriptorHeap は ImGui が GPU でテクスチャ参照を行うために ShaderVisible な CBV/SRV/UAV ヒープを 1 つ確保する。
//   * Begin/End はアプリケーション側のレンダリングループでフレーム毎に呼び出すこと。
//   * Draw() はコマンドリストを取得して SRV を含むヒープをセットし、ImGui のレンダリングを行う。
//   * Finalize() は ImGui のバックエンドをシャットダウンし、確保したヒープを解放する。
//

void ImGuiManager::Initialize(WinApp* winApp_)
{
	dxCommon_ = DirectXCommon::GetInstance();
	
	// デスクリプタヒープの設定（ImGui のテクスチャ SRV 用）
	// - ImGui_ImplDX12_Init に渡すため Shader-visible な CBV_SRV_UAV ヒープを 1 つ用意する。
	// - 必要に応じてデスクリプタ数を増やす（ここでは簡易に 1）。
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	// デスクリプタヒープの生成
	HRESULT result = dxCommon_->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(srvHeap_.GetAddressOf()));
	assert(SUCCEEDED(result));

#ifdef USE_IMGUI
	// ImGui のコンテキスト作成とバックエンド初期化 (Win32 + D3D12)
	// - ImGui_ImplDX12_Init の引数:
	//    * Device, バックバッファ数, RTV のフォーマット
	//    * DescriptorHeap, CPU/GPU の先頭ハンドル
	// - WinApp の HWND を ImGui_ImplWin32_Init に渡して Win32 イベント処理を結びつける
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp_->GetHwnd());
    ImGui_ImplDX12_Init(
		dxCommon_->GetDevice().Get(),
		static_cast<int>(dxCommon_->GetBackBufferCount()),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		srvHeap_.Get(),
		srvHeap_->GetCPUDescriptorHandleForHeapStart(),
		srvHeap_->GetGPUDescriptorHandleForHeapStart()
	);
#endif
}

void ImGuiManager::Begin()
{
#ifdef USE_IMGUI
	// フレーム毎の前処理:
	// - 各バックエンドに新フレーム開始を通知し、ImGui::NewFrame() を呼ぶ
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif
}

void ImGuiManager::End()
{
#ifdef USE_IMGUI
	// ImGui の描画データを生成（実際のレンダリングは Draw() で行う）
	ImGui::Render();
#endif
}

void ImGuiManager::Draw()
{
	// コマンドリストに対して ImGui 用の DescriptorHeap をセットしたうえで描画コマンドを発行する
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	// デスクリプタヒープ配列をコマンドリストにバインド
	ID3D12DescriptorHeap* ppHeaps[] = { srvHeap_.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

#ifdef USE_IMGUI
	// ImGui の描画コマンドを DirectX12 用実装に渡して発行する
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
#endif
}



void ImGuiManager::Finalize()
{
#ifdef USE_IMGUI
	// ImGui バックエンドのシャットダウン順序:
	// 1) D3D12 側の実装を終了
	// 2) Win32 側の実装を終了
	// 3) コンテキストを破棄
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif
	// 確保したデスクリプタヒープを解放
	srvHeap_.Reset();
}

#include "WinApp.h"
#include <wrl.h>
#include <imgui.h>
#include <cstdint>
#include <imgui_impl_win32.h>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#pragma comment(lib,"winmm.lib")


// ウィンドウプロシージャ
LRESULT CALLBACK WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
#ifdef USE_IMGUI
	// ImGuiにメッセージを渡す
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
#endif
	// メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		// ウィンドウが破棄された
	case WM_DESTROY:
		// OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	// 標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void WinApp::Initialize()
{
	// COMの初期化
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

	// システムタイマーの分解能を上げる
	timeBeginPeriod(1);
	///////////////////////
	// ウィンドウの作成
	///////////////////////

	//WNDCLASS wc_{};
	// ウィンドウプロシーシャ
	wc_.lpfnWndProc = WindowProc;
	// ウィンドウクラス名
	wc_.lpszClassName = L"GE3WindowClass";
	// インスタンスハンドル
	wc_.hInstance = GetModuleHandle(nullptr);
	// カーソル
	wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// ウィンドウクラスを登録する
	RegisterClass(&wc_);

	// ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0, 0, kClientWidth, kClientHeight };

	// クライアント領域を元に実際のサイズにrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウィンドウの生成
	hwnd_ = CreateWindow(
		// 利用するクラス名
		wc_.lpszClassName,
		// タイトルバーの文字
		L"GE3",
		// ウィンドウスタイル
		WS_OVERLAPPEDWINDOW,
		// 表示X座標
		CW_USEDEFAULT,
		// 表示Y座標
		CW_USEDEFAULT,
		// ウィンドウ横幅
		wrc.right - wrc.left,
		// ウィンドウ縦幅
		wrc.bottom - wrc.top,
		// 親ウィンドウハンドル
		nullptr,
		// メニューハンドル
		nullptr,
		// インスタンスハンドル
		wc_.hInstance,
		// オプション
		nullptr);

	// ウィンドウを表示する
	ShowWindow(hwnd_, SW_SHOW);
}



void WinApp::Finalize()
{
	CloseWindow(hwnd_);
	CoUninitialize();
}

bool WinApp::ProcessMessage()
{
	MSG msg{};
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (msg.message == WM_QUIT)
	{
		return true;
	}
	return false;
}

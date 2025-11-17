#pragma once
#include <windows.h>
#include <wrl.h>
#include <cstdint>
// WindowsAPI
class WinApp
{
public:	// 静的メンバ関数
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	// メンバ関数
	// 初期化
	void Initialize();
	

	/*------ゲッター------*/

	// ウィンドウハンドルの取得
	HWND GetHwnd() const { return hwnd; }

	// HINSTANCEの取得
	HINSTANCE GetHInstance() const { return wc.hInstance; }

	// 終了
	void Finalize();

	// メッセージの処理
	bool ProcessMessage();

	// 定数
	// クライアント領域のサイズ
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;
private:
	// ウィンドウハンドル
	HWND hwnd = nullptr;

	// ウィンドウクラスの設定
	WNDCLASS wc{};
};


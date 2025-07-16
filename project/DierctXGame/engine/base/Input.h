#pragma once
#include <windows.h>
#include <wrl.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "WinApp.h"

// 入力
class Input{
public:
	// namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	// シングルトンインスタンスの取得
	static Input* GetInstance() {
		static Input instance;
		return &instance;
	}

	Input() = default;
	~Input() = default;


	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;
	// メンバ変数
	// 初期化
	void Initialize(WinApp* winApp);
	// 更新
	void Update();

	/// <summary>
	/// キーの押下をチェック
	/// </summary>
	/// 押されているか
	bool PushKey(BYTE keyNumber);

	/// <summary>
	/// キーのトリガーをチェック
	/// </summary>
	bool TriggerKey(BYTE keyNumber);
private:
	
	

	// メンバ変数
	// キーボードのデバイス
	ComPtr<IDirectInputDevice8> keyboard;
	// 全キーの状態
	BYTE key[256] = {};
	// 前回の全キーの状態
	BYTE keyPre[256] = {};
	// DirectInputのインスタンス
	ComPtr<IDirectInput8> directInput = nullptr;

	// WindowsAPI
	WinApp* winApp_ = nullptr;
};
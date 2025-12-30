#include "Input.h"
#include <cassert>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

//
// Input
// - DirectInput8 を利用してキーボード入力を取得する簡易ラッパー実装。
// - 主な責務：DirectInput の初期化、毎フレームのキー状態取得、キー押下/トリガー判定を提供する。
// - 実装ノート：
//   - 本実装は DirectInput のキーボードデバイスを使用しており、Windows 固有の実装です。
//   - GetDeviceState は全キー（256 バイト）を一括で取得するため、個別キーのポーリングに最適化している。
//   - 前フレームの状態は `keyPre_` 配列で保持し、押下トリガー（押し始め）判定に利用する。
//   - エラーは assert で検出する実装になっているため、リリース時の堅牢性は必要に応じて強化すること。
//   - WinApp の HWND / HINSTANCE を利用して CooperativeLevel を設定するため、Initialize 前に WinApp を初期化しておくこと。
//

void Input::Initialize(WinApp* winApp) {
	// winApp の参照を保持（ウィンドウハンドル等の取得に利用）
	winApp_ = winApp;

	HRESULT result;
	// DirectInput インスタンスを作成
	result = DirectInput8Create(winApp->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);
	assert(SUCCEEDED(result));

	// キーボードデバイスを作成
	result = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(result));

	// データフォーマットをキーボードに設定（c_dfDIKeyboard を使用）
	result = keyboard_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	// 協調レベルを設定
	// - DISCL_FOREGROUND: フォアグラウンド時のみ取得
	// - DISCL_NONEXCLUSIVE: 他アプリと共有
	// - DISCL_NOWINKEY: Windows キーの無効化（必要に応じて外す）
	result = keyboard_->SetCooperativeLevel(winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}

void Input::Update() {
	// 前フレームのキー状態を保存しておく（トリガー検出用）
	memcpy(keyPre_, key_, sizeof(key_));

	// デバイス取得開始（Acquire は既に取得済でも安全に呼べる）
	keyboard_->Acquire();

	// 全キー状態を取得する（256 バイト）
	// - key_[i] != 0 はそのキーが押されていることを示す（High bit が立つ仕様）
	keyboard_->GetDeviceState(sizeof(key_), key_);
}

bool Input::PushKey(BYTE keyNumber)
{
	// 指定キーが押されているかを返す
	// 戻り値: 押されていれば true、押されていなければ false
	// 注意: key_ 配列の各要素は 0/非0 の値で押下状態を示す（非0 = 押下）
	if (key_[keyNumber]) {
		return true;
	}
	return false;
}

bool Input::TriggerKey(BYTE keyNumber)
{
	// 押下トリガー（押し始め）を検出する
	// - 前フレームは未押下で、今回フレームで押下されていれば true を返す
	if (!keyPre_[keyNumber] && key_[keyNumber]) {
		return true;
	}
	return false;
}

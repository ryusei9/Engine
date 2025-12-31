#pragma once
#include "DirectXCommon.h"

/// <summary>
/// モデル共通部
/// </summary>
class ModelCommon
{
public:
	// 初期化
	void Initialize(DirectXCommon* dxCommon);

	// ゲッター
	DirectXCommon* GetDxCommon() const { return dxCommon_; }
	
private:
	// メンバ変数
	DirectXCommon* dxCommon_ = nullptr;
};


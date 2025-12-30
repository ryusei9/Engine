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

	/*------ゲッター------*/
	DirectXCommon* GetDxCommon() const { return dxCommon_; }
	
private:
	DirectXCommon* dxCommon_;
};


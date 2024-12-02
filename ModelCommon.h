#pragma once
#include "DirectXCommon.h"
class ModelCommon
{
public:
	// 初期化
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// ゲッター
	/// </summary>
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	
private:
	DirectXCommon* dxCommon_;
};


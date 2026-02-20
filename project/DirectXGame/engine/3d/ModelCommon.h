#pragma once
#include "DirectXCommon.h"

using namespace MyEngine;
/// <summary>
/// モデル共通部
/// </summary>
class ModelCommon
{
public:
	// 初期化
	void Initialize(MyEngine::DirectXCommon* dxCommon);

	// ゲッター
	MyEngine::DirectXCommon* GetDxCommon() const { return dxCommon_; }
	
private:
	// メンバ変数
	MyEngine::DirectXCommon* dxCommon_ = nullptr;
};


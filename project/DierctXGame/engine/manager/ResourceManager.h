#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DepthMaterial.h>
#include <Object3dCommon.h>
class Camera;// 前方宣言

/// <summary>
/// リソースマネージャー
/// </summary>
class ResourceManager
{
public:
	// バッファリソースの作成
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);
};


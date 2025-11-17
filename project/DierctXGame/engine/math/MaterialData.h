#pragma once
#include <string>

// マテリアルデータ構造体
struct MaterialData {
	std::string textureFilePath;
	uint32_t textureIndex = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{};
};
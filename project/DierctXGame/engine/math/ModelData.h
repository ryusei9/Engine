#pragma once
#include <vector>
#include "MaterialData.h"
#include "VertexData.h"

// モデルデータ構造体
struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};
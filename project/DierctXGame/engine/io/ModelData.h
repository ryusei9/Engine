#pragma once
#include <vector>
#include "MaterialData.h"
#include "VertexData.h"

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};
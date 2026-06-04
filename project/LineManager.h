#pragma once

#include <vector>
#include <wrl.h>
#include <d3d12.h>

#include "Vector3.h"
#include "Vector4.h"

class Camera;

struct DebugLine
{
	Vector3 start;
	Vector3 end;
	Vector4 color;
};

struct LineVertex
{
	Vector3 position;
	Vector4 color;
};

class LineManager
{
public:

	static LineManager* GetInstance();

	void Initialize();

	void AddLine(const Vector3& start, const Vector3& end, const Vector4& color);

	void Draw();

	void Clear();

private:

	std::vector<DebugLine> lines_;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;

	D3D12_VERTEX_BUFFER_VIEW vbView_{};

	LineVertex* mappedVertices_ = nullptr;

	static const uint32_t kMaxLineCount = 2048;
};
#include "LineManager.h"
#include "DirectXCommon.h"

using namespace MyEngine;

LineManager* LineManager::GetInstance()
{
	static LineManager instance;
	return &instance;
}

void LineManager::Initialize()
{
	uint32_t vertexCount = kMaxLineCount * 2;

	uint32_t size = sizeof(LineVertex) * vertexCount;

	//vertexBuffer_ = DirectXCommon::GetInstance()->CreateBufferResource(size);

	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedVertices_));

	vbView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();

	vbView_.SizeInBytes = size;

	vbView_.StrideInBytes = sizeof(LineVertex);
}

void LineManager::AddLine(const Vector3& start, const Vector3& end, const Vector4& color)
{
	lines_.push_back({start, end, color});
}

void LineManager::Draw()
{
    if (lines_.empty()) {
        return;
    }

    uint32_t vertexIndex = 0;

    for (const auto& line : lines_) {

        mappedVertices_[vertexIndex++] = {line.start,line.color};

        mappedVertices_[vertexIndex++] = {line.end,line.color};
    }

    auto* commandList =DirectXCommon::GetInstance()->GetCommandList();

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    commandList->IASetVertexBuffers(0,1,&vbView_);

    commandList->DrawInstanced(vertexIndex,1,0,0);
}

void LineManager::Clear()
{
    lines_.clear();
}

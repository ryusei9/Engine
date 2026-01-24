#pragma once
#include "Matrix4x4.h"

// 正射影行列作成
namespace MakeOrthographicMatrix
{
	// 正射影行列
	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);
};


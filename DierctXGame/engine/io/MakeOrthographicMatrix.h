#pragma once
#include "Matrix4x4.h"
namespace MakeOrthographicMatrix
{
	// 正射影行列
	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);
};


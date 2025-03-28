#pragma once
#include <Matrix4x4.h>
#include <Vector3.h>
namespace MakeScaleMatrix {
	Matrix4x4 MakeScaleMatrix(const Vector3& scale)
	{
		Matrix4x4 result{};
		result.m[0][0] = scale.x;
		result.m[1][1] = scale.y;
		result.m[2][2] = scale.z;
		result.m[3][3] = 1.0f;
		return result;
	}
}
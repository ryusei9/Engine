#include "MakeRotateXMatrix.h"
#include "cmath"
namespace MakeRotateXMatrix {
	Matrix4x4 MakeRotateXMatrix(float radian)
	{
		Matrix4x4 resultRotateXMatrix = {};
		resultRotateXMatrix.m[0][0] = 1;
		resultRotateXMatrix.m[1][1] = std::cos(radian);
		resultRotateXMatrix.m[1][2] = std::sin(radian);
		resultRotateXMatrix.m[2][1] = -std::sin(radian);
		resultRotateXMatrix.m[2][2] = std::cos(radian);
		resultRotateXMatrix.m[3][3] = 1;
		return resultRotateXMatrix;
	}
}
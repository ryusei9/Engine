#include "MakeRotateZMatrix.h"
#include "cmath"
namespace MakeRotateZMatrix {
	Matrix4x4 MakeRotateZMatrix(float radian)
	{
		Matrix4x4 resultRotateZMatrix = {};
		resultRotateZMatrix.m[0][0] = std::cos(radian);
		resultRotateZMatrix.m[0][1] = std::sin(radian);
		resultRotateZMatrix.m[1][0] = -std::sin(radian);
		resultRotateZMatrix.m[1][1] = std::cos(radian);
		resultRotateZMatrix.m[2][2] = 1;
		resultRotateZMatrix.m[3][3] = 1;
		return resultRotateZMatrix;
	}
}
#include "MakeRotateYMatrix.h"
#include <cmath>

namespace MakeRotateYMatrix {
	Matrix4x4 MakeRotateYMatrix(float radian)
	{
		Matrix4x4 resultRotateYMatrix = {};
		resultRotateYMatrix.m[0][0] = std::cos(radian);
		resultRotateYMatrix.m[0][2] = -std::sin(radian);
		resultRotateYMatrix.m[1][1] = 1;
		resultRotateYMatrix.m[2][0] = std::sin(radian);
		resultRotateYMatrix.m[2][2] = std::cos(radian);
		resultRotateYMatrix.m[3][3] = 1;
		return resultRotateYMatrix;
	}
}
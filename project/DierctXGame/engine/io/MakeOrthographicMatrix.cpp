#include "MakeOrthographicMatrix.h"

namespace MakeOrthographicMatrix {
	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
		Matrix4x4 resultOrthographicMatrix = {};
		resultOrthographicMatrix.m[0][0] = 2 / (right - left);
		resultOrthographicMatrix.m[1][1] = 2 / (top - bottom);
		resultOrthographicMatrix.m[2][2] = 1 / (farClip - nearClip);
		resultOrthographicMatrix.m[3][0] = (left + right) / (left - right);
		resultOrthographicMatrix.m[3][1] = (top + bottom) / (bottom - top);
		resultOrthographicMatrix.m[3][2] = nearClip / (nearClip - farClip);
		resultOrthographicMatrix.m[3][3] = 1;
		return resultOrthographicMatrix;
	}
}
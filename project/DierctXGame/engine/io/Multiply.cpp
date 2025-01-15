#include "Multiply.h"

Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2)
{
	Matrix4x4 resultMultiply = {};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			resultMultiply.m[i][j] = m1.m[i][0] * m2.m[0][j] + m1.m[i][1] * m2.m[1][j] + m1.m[i][2] * m2.m[2][j] + m1.m[i][3] * m2.m[3][j];
		}
	}
	return resultMultiply;
}

Vector3 Multiply(const float& f, const Vector3& v)
{
	Vector3 resultMultiply = {};
	resultMultiply.x = f * v.x;
	resultMultiply.y = f * v.y;
	resultMultiply.z = f * v.z;
	return resultMultiply;
}

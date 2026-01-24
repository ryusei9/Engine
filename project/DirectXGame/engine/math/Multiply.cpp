#include "Multiply.h"
Matrix4x4 Multiply::Multiply(const Matrix4x4& m1, const Matrix4x4& m2)
{
	Matrix4x4 resultMultiply = {};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			resultMultiply.m[i][j] = m1.m[i][0] * m2.m[0][j] + m1.m[i][1] * m2.m[1][j] + m1.m[i][2] * m2.m[2][j] + m1.m[i][3] * m2.m[3][j];
		}
	}
	return resultMultiply;
}

// Matrix4x4とVector4の乗算
Vector4 Multiply::Multiply(const Matrix4x4& mat, const Vector4& vec)
{
	Vector4 result = {};
	result.x = mat.m[0][0] * vec.x + mat.m[1][0] * vec.y + mat.m[2][0] * vec.z + mat.m[3][0] * vec.w;
	result.y = mat.m[0][1] * vec.x + mat.m[1][1] * vec.y + mat.m[2][1] * vec.z + mat.m[3][1] * vec.w;
	result.z = mat.m[0][2] * vec.x + mat.m[1][2] * vec.y + mat.m[2][2] * vec.z + mat.m[3][2] * vec.w;
	result.w = mat.m[0][3] * vec.x + mat.m[1][3] * vec.y + mat.m[2][3] * vec.z + mat.m[3][3] * vec.w;
	return result;
}
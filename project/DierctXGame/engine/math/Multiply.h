#pragma once
#include "Matrix4x4.h"
#include "Vector4.h"

// 行列の掛け算
namespace Multiply
{
	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);
	Vector4 Multiply(const Matrix4x4& mat, const Vector4& vec);
};


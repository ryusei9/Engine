#pragma once
#include "Matrix4x4.h"
#include "Vector3.h"

// アフィン変換行列を作成する関数
namespace MakeAffineMatrix
{
	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3 translate);
};


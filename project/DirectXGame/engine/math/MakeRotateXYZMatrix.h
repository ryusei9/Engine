#pragma once
#include "MakeRotateXMatrix.h"
#include "MakeRotateYMatrix.h"
#include "MakeRotateZMatrix.h"
#include "Matrix4x4.h"
#include "Vector3.h"

// XYZ回転行列の作成
namespace MakeRotateXYZMatrix
{
	Matrix4x4 MakeRotateXYZMatrix(Vector3 rotate);
};


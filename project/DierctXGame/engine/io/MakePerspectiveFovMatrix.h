#pragma once
#include "Matrix4x4.h"
namespace MakePerspectiveFovMatrix
{
	Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
};


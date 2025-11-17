#pragma once
#include "Matrix4x4.h"

// 透視投影行列を作成する関数
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);



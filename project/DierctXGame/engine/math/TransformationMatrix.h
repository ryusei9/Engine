#pragma once
#include "Matrix4x4.h"

// 座標変換行列データ
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};
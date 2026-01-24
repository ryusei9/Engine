#pragma once
#include "Vector4.h"
#include "Vector3.h"

// ディレクショナルライト
struct DirectionalLight {
	Vector4 color; // ライトの色
	Vector3 direction; // ライトの向き
	float intensity; // 輝度
};
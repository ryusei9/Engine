#pragma once
#include "Vector3.h"
namespace Length
{
	// ベクトルの長さを計算
    float Length(const Vector3& v)
    {
        return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    }

};
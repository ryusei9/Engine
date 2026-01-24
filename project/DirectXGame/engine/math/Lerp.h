#pragma once
#include "Vector4.h"
#include <Vector3.h>

// 線形補間関数(Vector4)
inline Vector4 Lerp(const Vector4& a, const Vector4& b, float t) {
    return {
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.w + (b.w - a.w) * t
    };
}

// 線形補間関数(Vector3)
inline Vector3 Lerp(const Vector3& a, const Vector3& b, float t) {
    return {
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t
    };
}

// 線形補間関数(float)
inline float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

inline Vector3 CatmullRom(const Vector3& p0, const Vector3& p1,
    const Vector3& p2, const Vector3& p3, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;

    return (p1 * 2.0f +
        (p2 - p0) * t +
        (p0 * 2.0f - p1 * 5.0f + p2 * 4.0f - p3) * t2 +
        (-p0 + p1 * 3.0f - p2 * 3.0f + p3) * t3) * 0.5f;
}
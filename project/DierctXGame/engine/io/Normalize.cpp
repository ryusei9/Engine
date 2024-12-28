#include "Normalize.h"
#include <corecrt_math.h>


    Vector3 Normalize(const Vector3& v)
    {
		Vector3 NormalizeResult = {};
		float LengthResult = {};
		LengthResult = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
		NormalizeResult.x = v.x / LengthResult;
		NormalizeResult.y = v.y / LengthResult;
		NormalizeResult.z = v.z / LengthResult;
		return NormalizeResult;
    }

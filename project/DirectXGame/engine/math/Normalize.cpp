#include "Normalize.h"
#include <corecrt_math.h>
#include <Length.h>

namespace Normalize {
    Vector3 Normalize::Normalize(const Vector3& v)
    {
		Vector3 NormalizeResult = {};
		float LengthResult = Length::Length(v);
		
		NormalizeResult.x = v.x / LengthResult;
		NormalizeResult.y = v.y / LengthResult;
		NormalizeResult.z = v.z / LengthResult;
		return NormalizeResult;
    }
}
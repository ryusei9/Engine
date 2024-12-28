#include "Length.h"
#include <corecrt_math.h>

float Length(const Vector3& v)
{
	float LengthResult = {};
	LengthResult = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	return LengthResult;
}

#include "Add.h"

Vector3 Add(const Vector3& v1, const Vector3& v2)
{
	Vector3 AddResult = {};
	AddResult.x = v1.x + v2.x;
	AddResult.y = v1.y + v2.y;
	AddResult.z = v1.z + v2.z;
	return AddResult;
}

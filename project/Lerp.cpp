#include "Lerp.h"

float Lerp(float start, float end, float t)
{
    return start + t * (end - start);
}

Vector3 Lerp(const Vector3& start, const Vector3& end, float t)
{
    Vector3 result;
    result.x = Lerp(start.x, end.x, t);
    result.y = Lerp(start.y, end.y, t);
    result.z = Lerp(start.z, end.z, t);
    return result;
}
#include "MakeIdentity4x4.h"

namespace MakeIdentity4x4 {
    Matrix4x4 MakeIdentity4x4()
    {
		Matrix4x4 resultIdentity = {};
		for (int i = 0; i < 4; i++) {
			resultIdentity.m[i][i] = 1.0f;
		}
		return resultIdentity;
    }
}
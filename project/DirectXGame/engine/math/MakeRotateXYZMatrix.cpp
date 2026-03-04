#include "MakeRotateXYZMatrix.h"

namespace Math {
	Matrix4x4 MakeRotateXYZMatrix(Vector3 rotate)
	{
		Matrix4x4 rotateMatrix;
		return rotateMatrix = MakeRotateXMatrix(rotate.x) *
			MakeRotateYMatrix(rotate.y) *
			MakeRotateZMatrix(rotate.z);

	}
}
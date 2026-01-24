#include "MakeRotateXYZMatrix.h"

Matrix4x4 MakeRotateXYZMatrix::MakeRotateXYZMatrix(Vector3 rotate)
{
	Matrix4x4 rotateMatrix;
	return rotateMatrix = MakeRotateXMatrix::MakeRotateXMatrix(rotate.x) *
		MakeRotateYMatrix::MakeRotateYMatrix(rotate.y) *
		MakeRotateZMatrix::MakeRotateZMatrix(rotate.z);

}

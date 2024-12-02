#include "MakeAffineMatrix.h"
#include "Multiply.h"
#include "MakeRotateXMatrix.h"
#include "MakeRotateYMatrix.h"
#include "MakeRotateZMatrix.h"

namespace MakeAffineMatrix {
    Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3 translate)
    {
		Matrix4x4 resultAffineMatrix = {};
		Matrix4x4 resultRotateXYZMatrix = Multiply::Multiply(MakeRotateXMatrix::MakeRotateXMatrix(rotate.x), Multiply::Multiply(MakeRotateYMatrix::MakeRotateYMatrix(rotate.y), MakeRotateZMatrix::MakeRotateZMatrix(rotate.z)));
		resultAffineMatrix.m[0][0] = scale.x * resultRotateXYZMatrix.m[0][0];
		resultAffineMatrix.m[0][1] = scale.x * resultRotateXYZMatrix.m[0][1];
		resultAffineMatrix.m[0][2] = scale.x * resultRotateXYZMatrix.m[0][2];
		resultAffineMatrix.m[1][0] = scale.y * resultRotateXYZMatrix.m[1][0];
		resultAffineMatrix.m[1][1] = scale.y * resultRotateXYZMatrix.m[1][1];
		resultAffineMatrix.m[1][2] = scale.y * resultRotateXYZMatrix.m[1][2];
		resultAffineMatrix.m[2][0] = scale.z * resultRotateXYZMatrix.m[2][0];
		resultAffineMatrix.m[2][1] = scale.z * resultRotateXYZMatrix.m[2][1];
		resultAffineMatrix.m[2][2] = scale.z * resultRotateXYZMatrix.m[2][2];
		resultAffineMatrix.m[3][0] = translate.x;
		resultAffineMatrix.m[3][1] = translate.y;
		resultAffineMatrix.m[3][2] = translate.z;
		resultAffineMatrix.m[3][3] = 1;
		return resultAffineMatrix;
    }
}
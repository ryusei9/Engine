#pragma once
#include <Matrix4x4.h>
#include <Vector3.h>


// 平行移動行列の作成
namespace MakeTranslateMatrix {
	Matrix4x4 MakeTranslateMatrix(const Vector3& translate)
	{
		Matrix4x4 result{};
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				if (i == j)
				{
					result.m[i][j] = 1.0f;
				} else
				{
					result.m[i][j] = 0.0f;
				}
			}
		}
		result.m[3][0] = translate.x;
		result.m[3][1] = translate.y;
		result.m[3][2] = translate.z;
		return result;
	}
}
#pragma once
#include "Matrix4x4.h"
#include "Vector2.h"
#include "Vector3.h"
#include <cassert>
#include <cmath>
#include <corecrt_math_defines.h>

class MathMatrix {
public:
	// 行列の減法
	Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2);
	// 行列の積
	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

	// 平行移動行列
	Matrix4x4 MakeTranslate(const Vector3& translate);

	// 拡大縮小行列
	Matrix4x4 MakeScale(const Vector3& scale);

	// 座標変換
	Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);

	// x軸回転行列
	Matrix4x4 MakeRotateX(float radian);

	// y軸回転行列
	Matrix4x4 MakeRotateY(float radian);

	// z軸回転行列
	Matrix4x4 MakeRotateZ(float radian);

	// 3次元アフィン変換行列
	Matrix4x4 MakeAffine(const Vector3& scale, const Vector3& rotate, const Vector3 translate);

	// 正射影行列
	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

	// ビューポート変換行列
	Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

	// easeInの関数
	float easeInOut(float x);

	float lerp(float easeStart, float easeEnd, float t);

	Vector3 lerp(Vector3 easeStart, Vector3 easeEnd, float t);

	// Vector3の足し算
	Vector3 Add(Vector3 v1, Vector3 v2) { return Vector3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);}
	// 減算
	Vector3 Subtract(const Vector3 v1, const Vector3 v2);

	// ベクトル変換
	Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);

	// 正規化
	Vector3 Normalize(const Vector3 v);

	// スカラー倍
	Vector3 Multiply(const float scaler, const Vector3& v);

	// 長さ
	float Length(const Vector3& v);

	// 逆行列
	Matrix4x4 Inverse(const Matrix4x4& m);

	
};

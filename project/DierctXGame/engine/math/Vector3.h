#pragma once
#include <corecrt_math.h>
#include <Vector4.h>
#include <Matrix4x4.h>
#include <Multiply.h>

// 3次元ベクトル構造体
struct Vector3 {
	float x;
	float y;
	float z;

	// 加算演算子
	Vector3 operator+(const Vector3& other) const {
		return { x + other.x, y + other.y, z + other.z };
	}

	// 減算演算子
	Vector3 operator-(const Vector3& other) const {
		return { x - other.x, y - other.y, z - other.z };
	}

	// 積算演算子
	Vector3 operator*(const Vector3& other) const {
		return { x * other.x, y * other.y, z * other.z };
	}

	// スカラー乗算演算子
	Vector3 operator*(float scalar) const {
		return { x * scalar, y * scalar, z * scalar };
	}

	// スカラー除算演算子
	Vector3 operator/(float scalar) const {
		return { x / scalar, y / scalar, z / scalar };
	}

	// 加算代入演算子
	Vector3& operator+=(const Vector3& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	// 減算代入演算子
	Vector3& operator-=(const Vector3& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	// 積算代入演算子
	Vector3& operator*=(const Vector3& other) {
		x *= other.x;
		y *= other.y;
		z *= other.z;
		return *this;
	}

	// スカラー乗算代入演算子
	Vector3& operator*=(float scalar) {
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	// スカラー除算代入演算子
	Vector3& operator/=(float scalar) {
		x /= scalar;
		y /= scalar;
		z /= scalar;
		return *this;
	}

	// 単項演算子
	Vector3 operator-() const {
		return { -x, -y, -z };
	}

	// ベクトルの長さを計算する関数
	static float Length(const Vector3& vec) {
		return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
	}

	template<typename T>
	T Clamp(const T& v, const T& lo, const T& hi) {
		return (v < lo) ? lo : (hi < v) ? hi : v;
	}

	static Vector3 TransformCoord(const Vector3& v, const Matrix4x4& m) {
		Vector4 tmp = { v.x, v.y, v.z, 1.0f };
		Vector4 result = Multiply::Multiply(m, tmp);
		if (result.w != 0.0f) {
			result.x /= result.w;
			result.y /= result.w;
			result.z /= result.w;
		}
		return { result.x, result.y, result.z };
	}
};


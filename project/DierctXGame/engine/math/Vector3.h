#pragma once
#include <corecrt_math.h>

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
};


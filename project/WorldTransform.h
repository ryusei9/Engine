#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"
class WorldTransform
{
public:
	void Initialize();

	void UpdateMatrix();

	Vector3 scale_ = { 1.0f,1.0f,1.0f };
	Vector3 rotate_ = { 0.0f,0.0f,0.0f };
	Vector3 translate_ = { 0.0f,0.0f,0.0f };

	Matrix4x4 matWorld_;

	const WorldTransform* parent_ = nullptr;
};


#pragma once
#include "ViewProjection.h"
#include "MathMatrix.h"

// 前方宣言
class Player;

class CameraController {
public:
	/// <summary>
	///  初期化
	/// </summary>
	void initialize(ViewProjection* viewProjection);

	/// <summary>
	///  更新
	/// </summary>
	void Update();

	void SetTarget(Player* target) { target_ = target; }

	void Reset();

	// 矩形
	struct Rect {
		float left = 0.0f;
		float right = 1.0f;
		float bottom = 0.0f;
		float top = 1.0f;
	};

	void SetMovebleArea(Rect area) { movebleArea_ = area; }

private:
	ViewProjection* viewProjection_;
	Player* target_ = nullptr;
	// 追従対象とカメラの座標の差(オフセット)
	Vector3 targetOffset_ = {0.0f, 0.0f, -50.0f};
	

	// カメラ移動範囲
	Rect movebleArea_ = {0, 100, 0, 100};

	// カメラの目標座標
	Vector3 targetPos;

	// 座標補間割合
	static inline const float kInterpolationRate = 1.5f;

	MathMatrix* mathMatrix_;

	// 速度掛け算
	static inline const float kVelocityBias = 2.0f;

	// 追従対象の各方向へのカメラ移動範囲
	static inline const Rect margin = {-30.0f, 30.0f, -30.0f, 30.0f};
};

#pragma once
#include "Vector3.h"
#include "Input.h"
#include "Object3d.h"
class Player
{
public:
	/// メンバ関数
	/// <summary>
	///  初期化
	/// </summary>
	void Initialize(Object3d* model);

	/// <summary>
	///  更新
	/// </summary>
	void Update();

	/// <summary>
	///  描画
	/// </summary>
	void Draw();
private:
	/// メンバ変数
	struct WorldTransform {
		Vector3 scale;
		Vector3 rotate;
		Vector3 translate;
	};
	WorldTransform worldTranform;

	Input* input_;

	Object3d* model_;
};


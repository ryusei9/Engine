#pragma once
#include "Vector3.h"
class Player
{
public:
	/// メンバ関数
	/// <summary>
	///  初期化
	/// </summary>
	void Initialize();

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
		Vector3 transform;
	};
	WorldTransform worldTranform;
};


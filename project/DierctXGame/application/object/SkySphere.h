#pragma once
#include <Transform.h>
#include <Object3d.h>
class SkySphere
{
public:
	// 初期化
	void Initialize(Object3d* model);
	// 更新
	void Update();
	// 描画
	void Draw();
private:
	// 3Dオブジェクト
	Object3d* model_ = nullptr;
	// 移動
	Transform transform_;
};


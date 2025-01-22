#pragma once
#include "SRFramework.h"


// ゲーム全体
class MyGame : public SRFramework
{
public:

	// 初期化
	void Initialize() override;

	// 終了
	void Finelize() override;

	// 毎フレーム更新
	void Update() override;

	// 描画
	void Draw() override;


private:
	



	//Transform cameraTransform{
	//	{1.0f,1.0f,1.0f},
	//	{0.0f,0.0f,0.0f},
	//	{0.0f,0.0f,-10.0f}
	//};

	//bool useMonsterBall = true;

	//Vector2 spritePosition = { 100.0f,100.0f };

	//SoundData soundData1;

	//// ゲーム終了フラグ
	//bool endRequest_ = false;
};


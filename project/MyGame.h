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
	
	Sprite* sprite = new Sprite();

	// 入力の初期化
	Input* input = new Input();
	
	
	bool useMonsterBall = true;

	Vector2 spritePosition = { 100.0f,100.0f };

	SoundData soundData1;

	// ゲーム終了フラグ
	//bool endRequest_ = false;
};


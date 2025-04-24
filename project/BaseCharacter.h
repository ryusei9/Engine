#pragma once
#include <Object3d.h>
#include <Input.h>
#include <Camera.h>
// キャラクターの基底クラス
class BaseCharacter
{
public:
	virtual void Initialize();

	virtual void Update();

	virtual void Draw();

	// キャラクターの移動
	virtual void Move() = 0;

	// キャラクターの攻撃
	virtual void Attack() = 0;
protected:
	/*------メンバ変数------*/
	std::unique_ptr<Object3d> object3d; // 3Dオブジェクト

	// 入力
	std::unique_ptr<Input> input_ = nullptr;

	// カメラ
	Camera* camera_ = nullptr;

	// ヒットポイント
	int hp_ = 10;
};


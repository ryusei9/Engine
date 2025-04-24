#pragma once
#include <Object3d.h>
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

};


#pragma once
#include <DirectXCommon.h>
#include <Input.h>
#include <ModelManager.h>
#include <Object3dCommon.h>
#include <SpriteCommon.h>
#include <Object3d.h>
#include <Sprite.h>
class GameScene
{
public:
	~GameScene();
	/// 初期化
	void Initialize();

	/// 更新
	void Update();

	/// 描画
	void Draw();
private:
	// 基盤
	DirectXCommon* dxCommon_;

	Input* input_;

	Object3dCommon* object3dCommon_;

	SpriteCommon* spriteCommon_;

	Camera* camera_;

	std::vector<Sprite*> sprites;

	std::vector<Object3d*> object3ds;

	// ゲームに関する変数

	Transform cameraTransform{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,-10.0f}
	};
};


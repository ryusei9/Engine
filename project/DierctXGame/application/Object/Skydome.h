#pragma once
#include <WorldTransform.h>
#include <Object3d.h>
#include <Camera.h>
class Skydome
{
public:
	// 初期化
	void Initialize();
	// 更新
	void Update();
	// 描画
	void Draw();
	// ImGuiでの設定
	void DrawImGui();
	// セッター
	void SetWorldTransform(const WorldTransform& worldTransform) { this->worldTransform_ = worldTransform; }
	void SetCamera(Camera* camera) { this->camera_ = camera; }
private:
	std::unique_ptr<Object3d> object3d_; // 3Dオブジェクト

	WorldTransform worldTransform_; // ワールド変換

	Camera* camera_ = nullptr; // カメラ
	
	
};


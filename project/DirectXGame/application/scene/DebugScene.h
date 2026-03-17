#pragma once
#include "BaseScene.h"
#include <Player.h>
#include <Enemy.h>
#include <MiniBoss.h>
#include <Input.h>
#include <BaseScene.h>

class DebugScene : public BaseScene
{
	public:
	// 初期化
	void Initialize(DirectXCommon* directXCommon, WinApp* winApp) override;
	// 更新
	void Update() override;
	// 描画
	void Draw() override;
	// 終了
	void Finalize() override;
	// ImGui描画
	void DrawImGui() override;
private:
	// プレイヤー
	std::unique_ptr<Player> player_ = nullptr;

	// ミニボス
	std::unique_ptr<MiniBoss> miniBoss_ = nullptr;
};


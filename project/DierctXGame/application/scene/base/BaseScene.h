#pragma once
#include <SpriteCommon.h>
#include <DirectXCommon.h>
#include <WinApp.h>
// シーンの種類
enum SCENE { TITLE, GAMEPLAY };
/// <summary>
/// シーン基底クラス
/// </summary>
class BaseScene
{
public:
	// デストラクタ
	virtual ~BaseScene() = default;

	// 初期化
	virtual void Initialize(DirectXCommon* directXCommon, WinApp* winApp) = 0;

	// 更新
	virtual void Update() = 0;

	// 描画
	virtual void Draw() = 0;

	// 終了
	virtual void Finalize() = 0;

	// ImGui描画
	virtual void DrawImGui() = 0;

	// シーン番号の取得
	int GetSceneNo() { return sceneNo; }

	// シーン番号の設定
	void SetSceneNo(int sceneNo) { this->sceneNo = sceneNo; }

public:
	// シーン番号
	static int sceneNo;
};


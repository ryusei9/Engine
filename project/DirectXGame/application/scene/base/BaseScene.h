#pragma once
#include <SpriteCommon.h>
#include <DirectXCommon.h>
#include <WinApp.h>
#include <cstdint>
#include <optional>
using namespace MyEngine;
// シーンの種類（意味付けのためenumを明示）
enum SCENE { TITLE, GAMEPLAY, GAMEOVER,DEBUG };

/// <summary>
/// シーン基底クラス
/// - ゲームシーンの共通インターフェースを提供
/// - 初期化・更新・描画・終了の一連のライフサイクルを定義
/// - DirectXCommon, WinAppに依存
/// - シーン番号はSceneManagerで管理され、各シーンは遷移要求のみを持つ
/// </summary>
class BaseScene
{
public:
	// デストラクタ
	virtual ~BaseScene() = default;

	/// <summary>
	/// 初期化処理（純粋仮想関数）
	/// </summary>
	/// <param name="directXCommon">DirectXCommonインスタンス</param>
	/// <param name="winApp">WinAppインスタンス</param>
	virtual void Initialize(DirectXCommon* directXCommon, WinApp* winApp) = 0;

	/// <summary>
	/// 更新処理（純粋仮想関数）
	/// </summary>
	virtual void Update() = 0;

	/// <summary>
	/// 描画処理（純粋仮想関数）
	/// </summary>
	virtual void Draw() = 0;

	/// <summary>
	/// 終了処理（純粋仮想関数）
	/// </summary>
	virtual void Finalize() = 0;

	/// <summary>
	/// ImGui描画処理（純粋仮想関数）
	/// </summary>
	virtual void DrawImGui() = 0;

	/// <summary>
	/// 次のシーンを取得
	/// - シーン遷移が必要な場合は次のシーン番号を返す
	/// - 遷移不要な場合はstd::nulloptを返す
	/// </summary>
	/// <returns>次のシーン番号（遷移なしの場合はstd::nullopt）</returns>
	std::optional<int32_t> GetNextScene() const { return nextScene_; }

protected:
	/// <summary>
	/// 次のシーンを要求
	/// - 派生クラスでシーン遷移時にこのメソッドを呼び出す
	/// </summary>
	/// <param name="sceneNo">遷移先のシーン番号</param>
	void RequestSceneChange(int32_t sceneNo) { nextScene_ = sceneNo; }

private:
	// 次のシーン番号（遷移要求がない場合はnullopt）
	std::optional<int32_t> nextScene_ = std::nullopt;
};


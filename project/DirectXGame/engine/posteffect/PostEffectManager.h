#pragma once
#include <memory>
#include <PostEffectBase.h>
#include <SrvManager.h>

/// <summary>
/// ポストエフェクトマネージャー
/// </summary>
class PostEffectManager
{
public:
	// 初期化
	void Initialize(DirectXCommon* dxCommon);

	// ポストエフェクトを追加する
	void AddEffect(std::unique_ptr<PostEffectBase> effect);

	// ポストエフェクトのインデックスを指定して有効・無効を設定する
	void SetEffectEnabled(size_t index, bool enebled);

	// 有効なエフェクトだけ順番にPreRender
	void PreRenderAll();

	// 有効なエフェクトだけ順番にDraw
	void DrawAll();

	// 有効なエフェクトだけ順番にPostRender
	void PostRenderAll();

	// 時間パラメータを設定
	void SetTimeParams(float time);

	// 有効なエフェクトだけ順番にPreBarrier
	void PreBarrierAll();

	// 有効なエフェクトだけ順番にPostBarrier
	void PostBarrierAll();

	// ImGui描画
	void DrawImGui();

private:
	DirectXCommon* dxCommon_ = nullptr;
	std::vector<std::unique_ptr<PostEffectBase>> effects_;
	std::vector<bool> enabled_;
	SrvManager* srvManager_ = nullptr;
};


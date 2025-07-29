#pragma once
#include <memory>
#include <PostEffectBase.h>
#include <SrvManager.h>
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

	void SetTimeParams(float time);

	// 有効なエフェクトだけ順番にPreBarrier
	void PreBarrierAll();

	// 有効なエフェクトだけ順番にPostBarrier
	void PostBarrierAll();

	void DrawImGui();

private:
	DirectXCommon* dxCommon_ = nullptr;
	std::vector<std::unique_ptr<PostEffectBase>> effects_;
	std::vector<bool> enabled_;
	SrvManager* srvManager_ = nullptr;
};


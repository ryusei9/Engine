#include "FadeManager.h"
#include <imgui.h>
#include "DirectXCommon.h"

//
// FadeManager
// - 画面フェード（フェードイン / フェードアウト）を管理するユーティリティクラス。
// - 内部で透過スプライトを保持し、alpha 値を時間経過で増減させることでフェード演出を実現する。
// - 状態機械（EffectState）を持ち、FadeIn / FadeOut / Finish / None を遷移する。
// - 使用例:
//     FadeManager mgr;
//     mgr.Initialize();
//     mgr.FadeInStart(0.02f, [](){ /* フェードイン完了時の処理 */ });
//     // 毎フレーム: mgr.Update(); mgr.Draw();
// - 注意点:
//   - fadeSpeed は毎フレーム加算/減算する量（フレームレート固定想定）。デルタタイム対応にしたい場合は外部から dt を渡す設計に変更すること。
//   - onFinished コールバックはフェード完了時に呼ばれる。短いラムダを渡すか、nullptr の可能性を考慮すること。
//   - スプライトは内部で生成して Upload / GPU へ配置するため、Initialize を必ず呼ぶこと。
//

using namespace FadeManagerConstants;

void FadeManager::Initialize()
{
	// フェード用スプライトを生成・初期化する
	fadeSprite_ = std::make_unique<Sprite>();
	fadeSprite_->Initialize(DirectXCommon::GetInstance(), kFadeTextureFilePath);
	fadeSprite_->SetColor({ kDefaultColorR, kDefaultColorG, kDefaultColorB, alpha_ });
}

void FadeManager::Update()
{
	// 状態に応じてアルファ値を更新
	if (fadeState_ == EffectState::FadeIn) {
		UpdateFadeIn();
	} else if (fadeState_ == EffectState::FadeOut) {
		UpdateFadeOut();
	}

	// スプライトのアルファを常に反映
	UpdateSpriteColor();
	fadeSprite_->Update();
}

void FadeManager::Draw()
{
	// 描画が必要な場合のみ描画
	if (ShouldDraw()) {
		fadeSprite_->Draw();
	}
}

void FadeManager::FadeInStart(float fadeSpeed, std::function<void()> onFinished)
{
	// フェードインを開始
	fadeState_ = EffectState::FadeIn;
	fadeSpeed_ = fadeSpeed;
	alpha_ = kMinAlpha;
	onFinished_ = onFinished;
}

void FadeManager::FadeOutStart(float fadeSpeed, std::function<void()> onFinished)
{
	// フェードアウトを開始
	fadeState_ = EffectState::FadeOut;
	fadeSpeed_ = fadeSpeed;
	alpha_ = kMaxAlpha;
	onFinished_ = onFinished;
}

void FadeManager::DrawImGui()
{
#ifdef USE_IMGUI
	ImGui::Begin("FadeManager");
	
	// 状態表示
	const char* stateStr = "";
	switch (fadeState_) {
		case EffectState::None:    stateStr = "None"; break;
		case EffectState::FadeIn:  stateStr = "FadeIn"; break;
		case EffectState::FadeOut: stateStr = "FadeOut"; break;
		case EffectState::Finish:  stateStr = "Finish"; break;
	}
	ImGui::Text("Fade State: %s", stateStr);

	// パラメータ調整
	ImGui::SliderFloat("Alpha", &alpha_, kMinAlpha, kMaxAlpha);
	ImGui::SliderFloat("Fade Speed", &fadeSpeed_, kImGuiSpeedMin, kImGuiSpeedMax);
	
	ImGui::End();
#endif
}

// ===== ヘルパー関数 =====

void FadeManager::UpdateFadeIn()
{
	alpha_ += fadeSpeed_;
	
	if (alpha_ >= kMaxAlpha) {
		alpha_ = kMaxAlpha;
		OnFadeComplete();
	}
}

void FadeManager::UpdateFadeOut()
{
	alpha_ -= fadeSpeed_;
	
	if (alpha_ <= kMinAlpha) {
		alpha_ = kMinAlpha;
		OnFadeComplete();
	}
}

void FadeManager::UpdateSpriteColor()
{
	fadeSprite_->SetColor({ kDefaultColorR, kDefaultColorG, kDefaultColorB, alpha_ });
}

void FadeManager::ClampAlpha()
{
	if (alpha_ < kMinAlpha) {
		alpha_ = kMinAlpha;
	} else if (alpha_ > kMaxAlpha) {
		alpha_ = kMaxAlpha;
	}
}

void FadeManager::OnFadeComplete()
{
	fadeState_ = EffectState::Finish;
	
	if (onFinished_) {
		onFinished_();
	}
}

bool FadeManager::ShouldDraw() const
{
	// フェード中、または完了状態でアルファが残っている場合に描画
	return fadeState_ == EffectState::FadeIn || 
	       fadeState_ == EffectState::FadeOut ||
	       (fadeState_ == EffectState::Finish && alpha_ > kMinAlpha);
}
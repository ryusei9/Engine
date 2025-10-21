#include "FadeManager.h"
#include <imgui.h>


void FadeManager::Initialize()
{
	fadeSprite_ = std::make_unique<Sprite>();
	fadeSprite_->Initialize(DirectXCommon::GetInstance(), "resources/fadeWhite.png");
	fadeSprite_->SetColor({ 1.0f, 1.0f, 1.0f, alpha_ });
}


void FadeManager::Update(float deltaTime)
{
	// Fadeの状態に応じて処理を行う
	if (fadeState_ == EffectState::FadeIn) {
		alpha_ += deltaTime * fadeSpeed_;
		if (alpha_ >= 1.0f) {
			alpha_ = 1.0f;
			fadeState_ = EffectState::Finish;
			if (onFinished_) {
				onFinished_();
			}
		}
	} else if (fadeState_ == EffectState::FadeOut) {
		alpha_ -= deltaTime * fadeSpeed_;
		if (alpha_ <= 0.0f) {
			alpha_ = 0.0f;
			fadeState_ = EffectState::Finish;
			if (onFinished_) {
				onFinished_();
			}
		}
	}
	fadeSprite_->SetColor({ 1.0f, 1.0f, 1.0f, alpha_ });
	fadeSprite_->Update();
}



void FadeManager::Draw()
{
	if (fadeState_ == EffectState::FadeIn || fadeState_ == EffectState::FadeOut ||
		(fadeState_ == EffectState::Finish && alpha_ > 0.0f)) {
		fadeSprite_->Draw();
	}
}

void FadeManager::FadeInStart(float fadeSpeed)
{
	fadeState_ = EffectState::FadeIn;
	fadeSpeed_ = fadeSpeed;
	alpha_ = 0.0f;
}

void FadeManager::FadeOutStart(float fadeSpeed)
{
	fadeState_ = EffectState::FadeOut;
	fadeSpeed_ = fadeSpeed;
	alpha_ = 1.0f;
}

void FadeManager::DrawImGui()
{
	ImGui::Begin("FadeManager");
	const char* stateStr = "";
	switch (fadeState_) {
	case EffectState::None: stateStr = "None"; break;
	case EffectState::FadeIn: stateStr = "FadeIn"; break;
	case EffectState::FadeOut: stateStr = "FadeOut"; break;
	case EffectState::Finish: stateStr = "Finish"; break;
	}
	ImGui::Text("Fade State: %s", stateStr);
	ImGui::SliderFloat("Alpha", &alpha_, 0.0f, 1.0f);
	ImGui::SliderFloat("Fade Speed", &fadeSpeed_, 0.01f, 1.0f);
	ImGui::End();
}
#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include "Sprite.h"

// 演出管理クラス
class FadeManager
{
public:
    enum class EffectState {
        None,
        FadeIn,
        FadeOut,
        Finish,
    };
    void Initialize();

    void Update();

    void Draw();

    void FadeInStart(float fadeSpeed, std::function<void()> onFinished = nullptr);

	void FadeOutStart(float fadeSpeed, std::function<void()> onFinished = nullptr);

	void DrawImGui();

    
	EffectState GetFadeState() const { return fadeState_; }
private:
    std::unique_ptr<Sprite> fadeSprite_;

	EffectState fadeState_ = EffectState::None;

	float alpha_ = 0.0f;

	float fadeSpeed_ = 0.02f; // フェード速度

    std::function<void()> onFinished_ = nullptr;

};


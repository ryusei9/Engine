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

    void Update(float deltaTime);

    void Draw();

    void FadeInStart(float fadeSpeed);

	void FadeOutStart(float fadeSpeed);

	void DrawImGui();

    
	EffectState GetFadeState() const { return fadeState_; }
private:
    std::unique_ptr<Sprite> fadeSprite_;

	EffectState fadeState_ = EffectState::None;

	float alpha_ = 0.0f;

	float fadeSpeed_ = 0.5f; // フェード速度

    std::function<void()> onFinished_ = nullptr;

};


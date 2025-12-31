#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
#include "Sprite.h"

// FadeManager用の定数
namespace FadeManagerConstants {
	// デフォルトのフェード速度
	constexpr float kDefaultFadeSpeed = 0.02f;
	
	// アルファ値の範囲
	constexpr float kMinAlpha = 0.0f;
	constexpr float kMaxAlpha = 1.0f;
	
	// デフォルトの色（白）
	constexpr float kDefaultColorR = 1.0f;
	constexpr float kDefaultColorG = 1.0f;
	constexpr float kDefaultColorB = 1.0f;
	
	// テクスチャパス
	constexpr const char* kFadeTextureFilePath = "resources/fadeWhite.png";
	
	// ImGuiスライダーの範囲
	constexpr float kImGuiSpeedMin = 0.01f;
	constexpr float kImGuiSpeedMax = 1.0f;
}

/// <summary>
/// フェードエフェクト管理クラス
/// </summary>
class FadeManager
{
public:
	// フェード状態
	enum class EffectState {
		None,
		FadeIn,
		FadeOut,
		Finish,
	};

	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	// フェードイン開始
	void FadeInStart(float fadeSpeed, std::function<void()> onFinished = nullptr);

	// フェードアウト開始
	void FadeOutStart(float fadeSpeed, std::function<void()> onFinished = nullptr);

	// ImGui描画
	void DrawImGui();

	// ゲッター
	EffectState GetFadeState() const { return fadeState_; }
	float GetAlpha() const { return alpha_; }
	float GetFadeSpeed() const { return fadeSpeed_; }

private:
	// フェード更新ヘルパー
	void UpdateFadeIn();
	void UpdateFadeOut();
	void UpdateSpriteColor();
	
	// アルファ値のクランプ
	void ClampAlpha();
	
	// フェード完了処理
	void OnFadeComplete();
	
	// 描画判定
	bool ShouldDraw() const;

	// スプライト
	std::unique_ptr<Sprite> fadeSprite_;

	// フェード状態
	EffectState fadeState_ = EffectState::None;

	// アルファ値
	float alpha_ = FadeManagerConstants::kMinAlpha;

	// フェード速度
	float fadeSpeed_ = FadeManagerConstants::kDefaultFadeSpeed;

	// 完了時コールバック
	std::function<void()> onFinished_ = nullptr;
};


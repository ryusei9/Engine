#include "FadeManager.h"
#include <imgui.h>

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

void FadeManager::Initialize()
{
	// フェード用スプライトを生成・初期化する
	// - 使用するテクスチャは "resources/fadeWhite.png"（透過白）を想定
	// - 初期アルファ値 alpha_ を反映した色でスプライトの色をセットしておく
	fadeSprite_ = std::make_unique<Sprite>();
	fadeSprite_->Initialize(DirectXCommon::GetInstance(), "resources/fadeWhite.png");
	fadeSprite_->SetColor({ 1.0f, 1.0f, 1.0f, alpha_ });
}

void FadeManager::Update()
{
	// 状態に応じて alpha_ を増減し、状態遷移と完了コールバックを処理する
	//
	// 動作仕様:
	// - FadeIn: alpha を増加させ、1.0 に到達したら Finish に遷移して onFinished_ を呼ぶ
	// - FadeOut: alpha を減少させ、0.0 に到達したら Finish に遷移して onFinished_ を呼ぶ
	// - Finish / None: alpha の更新は行わない（必要なら外部から値を操作できる）
	//
	// 実装メモ:
	// - fadeSpeed_ はフレーム固定の増分として扱う（例: 0.02 => 約50フレームで変化）
	// - より滑らかな時間制御をしたければ引数で deltaTime を受け取るように拡張する
	if (fadeState_ == EffectState::FadeIn) {
		alpha_ += fadeSpeed_;
		if (alpha_ >= 1.0f) {
			alpha_ = 1.0f;
			fadeState_ = EffectState::Finish;
			if (onFinished_) onFinished_();
		}
	} else if (fadeState_ == EffectState::FadeOut) {
		alpha_ -= fadeSpeed_;
		if (alpha_ <= 0.0f) {
			alpha_ = 0.0f;
			fadeState_ = EffectState::Finish;
			if (onFinished_) onFinished_();
		}
	}

	// スプライトのアルファを常に反映して更新
	fadeSprite_->SetColor({ 1.0f, 1.0f, 1.0f, alpha_ });
	fadeSprite_->Update();
}

void FadeManager::Draw()
{
	// 描画条件:
	// - フェード中（FadeIn/FadeOut）のとき常に描画する
	// - 完了状態 (Finish) でも alpha_ > 0.0f であれば描画する（フェードイン完了直後に画面が白いまま残る場合など）
	// - None のときは描画しない
	if (fadeState_ == EffectState::FadeIn || fadeState_ == EffectState::FadeOut ||
		(fadeState_ == EffectState::Finish && alpha_ > 0.0f)) {
		fadeSprite_->Draw();
	}
}

void FadeManager::FadeInStart(float fadeSpeed, std::function<void()> onFinished)
{
	// フェードインを開始する
	// - fadeSpeed: 毎フレームの alpha 増分（正の値）
	// - onFinished: フェード完了時に呼ばれるコールバック（nullptr 可）
	fadeState_ = EffectState::FadeIn;
	fadeSpeed_ = fadeSpeed;
	alpha_ = 0.0f;
	onFinished_ = onFinished;
}

void FadeManager::FadeOutStart(float fadeSpeed, std::function<void()> onFinished)
{
	// フェードアウトを開始する
	// - fadeSpeed: 毎フレームの alpha 減分（正の値）
	// - onFinished: フェード完了時に呼ばれるコールバック（nullptr 可）
	fadeState_ = EffectState::FadeOut;
	fadeSpeed_ = fadeSpeed;
	alpha_ = 1.0f;
	onFinished_ = onFinished;
}

void FadeManager::DrawImGui()
{
#ifdef USE_IMGUI
	// デバッグ用 ImGui: フェード状態とパラメータを表示・編集できる
	ImGui::Begin("FadeManager");
	const char* stateStr = "";
	switch (fadeState_) {
	case EffectState::None: stateStr = "None"; break;
	case EffectState::FadeIn: stateStr = "FadeIn"; break;
	case EffectState::FadeOut: stateStr = "FadeOut"; break;
	case EffectState::Finish: stateStr = "Finish"; break;
	}
	ImGui::Text("Fade State: %s", stateStr);

	// alpha_ / fadeSpeed_ を直接操作できる（デバッグ用）
	ImGui::SliderFloat("Alpha", &alpha_, 0.0f, 1.0f);
	ImGui::SliderFloat("Fade Speed", &fadeSpeed_, 0.01f, 1.0f);
	ImGui::End();
#endif
}
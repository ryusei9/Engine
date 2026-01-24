#include "PostEffectManager.h"
#include <imgui.h>

//
// PostEffectManager
// - 複数のポストエフェクトパス(PostEffectBase 派生)を管理し、フレーム毎に順に実行するユーティリティクラス。
// - 主な責務：
//   * 各ポストエフェクトの初期化は外部で行われる前提（Initialize は dxCommon を保持するのみ）
//   * エフェクトの有効/無効管理（SRV/RTV へのリソース状態遷移を含む）
//   * 全エフェクトに対する PreRender / Draw / PostRender の呼び出しと、描画前後のバリア管理
//   * エフェクトに渡す共通パラメータ（例: 時刻）の配布
//
// 使用上の注意：
// - effects_ は unique_ptr で所有し、ライフタイムはこのマネージャに依存する。
// - enabled_ は effects_ と同じ長さを保つことを期待する（AddEffect で常に push_back している）
// - リソース状態遷移は各効果の Transition... メソッドに委譲しており、ここでは適切なタイミングで呼び出す。
// - ImGui による切り替えはスレッド非対応なのでメインスレッドで行うこと。
//


void PostEffectManager::Initialize(DirectXCommon* dxCommon) {
    // DirectXCommon を保存（描画で必要なハンドル等を取得するため）
    dxCommon_ = dxCommon;
}

void PostEffectManager::AddEffect(std::unique_ptr<PostEffectBase> effect) {
    // 新しいエフェクトを追加して有効フラグ配列を同期する
    // 入力: effect - 所有権を渡す unique_ptr
    // 副作用: effects_ に move で追加、enabled_ に true を追加（追加時は「有効」に設定）
    effects_.push_back(std::move(effect));
    enabled_.push_back(true);
}

void PostEffectManager::SetEffectEnabled(size_t index, bool enabled) {
    // 指定インデックスのエフェクトを有効/無効に切り替える
    // - index が範囲内なら状態を切り替え、状態変化に応じて当該エフェクトのレンダーテクスチャを
    //   SRV<->RTV に遷移させる（無効化時は SRV、 有効化時は RTV）。
    if (index < enabled_.size()) {
        // 無効化（有効 -> 無効）: 描画を終了して SRV として使用可能にする
        if (enabled_[index] && !enabled && effects_[index]) {
            effects_[index]->TransitionRenderTextureToShaderResource();
        }
        // 有効化（無効 -> 有効）: 描画先として使えるように RTV に遷移する
        if (!enabled_[index] && enabled && effects_[index]) {
            effects_[index]->TransitionRenderTextureToRenderTarget();
        }
        enabled_[index] = enabled;
    }
}

void PostEffectManager::PreRenderAll() {
    // 全エフェクトの PreRender を順に呼ぶ
    // - 各エフェクトの描画前処理（RTV バインド・クリア・ビューポート設定等）を行う
    for (size_t i = 0; i < effects_.size(); ++i) {
        if (enabled_[i] && effects_[i]) {
            effects_[i]->PreRender();
        }
    }
}

void PostEffectManager::DrawAll() {
    // 全エフェクトの Draw を実行
    // - 事前に PreBarrierAll を呼んでおくことで、すべてのエフェクト出力を SRV 状態に揃える（入力として使えるようにする）
    // - 各エフェクトは内部で自分の SRV を参照してフルスクリーン描画を行う
    PreBarrierAll(); // SRV バリアを張る

    // ここで currentInputSRV を取得しておくパターンもあるが、各エフェクト内で固定インデックスを参照する実装に依存する
    D3D12_GPU_DESCRIPTOR_HANDLE currentInputSRV = dxCommon_->GetSRVGPUDescriptorHandle(0); // 0: シーンの出力（例）

    for (size_t i = 0; i < effects_.size(); ++i) {
        if (enabled_[i] && effects_[i]) {
            effects_[i]->Draw();
        }
    }
}

void PostEffectManager::PostRenderAll() {
    // 全エフェクトの PostRender を順に呼ぶ
    // - 各エフェクトの描画後処理（出力リソースを SRV に遷移する等）を行う
    for (size_t i = 0; i < effects_.size(); ++i) {
        if (enabled_[i] && effects_[i]) {
            effects_[i]->PostRender();
        }
    }
}

void PostEffectManager::SetTimeParams(float time) {
    // 全エフェクトに共通の時間パラメータを配布する（ノイズや時間依存エフェクト向け）
    for (size_t i = 0; i < effects_.size(); ++i) {
        if (effects_[i]) {
            effects_[i]->SetTimeParams(time);
        }
    }
}

void PostEffectManager::PreBarrierAll()
{
    // 全エフェクトの出力を SRV 状態に遷移する（Draw の前に入力として扱えるようにする）
    // - これは複数エフェクト間で出力を連鎖的に参照する際に必要な同期処理
    for (size_t i = 0; i < effects_.size(); ++i) {
        if (enabled_[i] && effects_[i]) {
            effects_[i]->TransitionRenderTextureToShaderResource();
        }
	}
}

void PostEffectManager::PostBarrierAll()
{
    // 全エフェクトの出力を RTV 状態に遷移する（再びレンダーターゲットとして書き込み可能にする）
    for (size_t i = 0; i < effects_.size(); ++i) {
        if (enabled_[i] && effects_[i]) {
            effects_[i]->TransitionRenderTextureToRenderTarget();
        }
	}
}

void PostEffectManager::DrawImGui() {  
#ifdef USE_IMGUI
    // デバッグ用 UI:
    // - 各エフェクトの ON/OFF を切り替えられるチェックボックスを表示する
    // - 切り替え時に適切なリソース遷移を行う
    ImGui::Begin("Post Effects");
    for (size_t i = 0; i < effects_.size(); ++i) {
        if (effects_[i]) {
            bool enabled = enabled_[i];
            if (ImGui::Checkbox(effects_[i]->GetName(), &enabled)) {
                // 無効化時は SRV へ遷移
                if (enabled_[i] && !enabled) {
                    effects_[i]->TransitionRenderTextureToShaderResource();
                }
                // 有効化時は RTV へ遷移
                if (!enabled_[i] && enabled) {
                    effects_[i]->TransitionRenderTextureToRenderTarget();
                }
                enabled_[i] = enabled;
            }
        }
    }
    ImGui::End();
#endif
}

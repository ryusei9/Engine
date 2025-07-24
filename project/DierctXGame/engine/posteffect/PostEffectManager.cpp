#include "PostEffectManager.h"
#include <imgui.h>

void PostEffectManager::Initialize(DirectXCommon* dxCommon) {
    dxCommon_ = dxCommon;
}

void PostEffectManager::AddEffect(std::unique_ptr<PostEffectBase> effect) {
    effects_.push_back(std::move(effect));
    enabled_.push_back(true); // 追加時は無効化
}

void PostEffectManager::SetEffectEnabled(size_t index, bool enabled) {
    if (index < enabled_.size()) {
        // 無効化時はSRVへ
        if (enabled_[index] && !enabled && effects_[index]) {
            effects_[index]->TransitionRenderTextureToShaderResource();
        }
        // 有効化時はRTVへ
        if (!enabled_[index] && enabled && effects_[index]) {
            effects_[index]->TransitionRenderTextureToRenderTarget();
        }
        enabled_[index] = enabled;
    }
}

void PostEffectManager::PreRenderAll() {
    for (size_t i = 0; i < effects_.size(); ++i) {
        if (enabled_[i] && effects_[i]) {
            effects_[i]->PreRender();
        }
    }
}

void PostEffectManager::DrawAll() {
    PreBarrierAll(); // 追加: SRVバリアを張る
    D3D12_GPU_DESCRIPTOR_HANDLE currentInputSRV = dxCommon_->GetSRVGPUDescriptorHandle(0); // 0: シーンの出力
    for (size_t i = 0; i < effects_.size(); ++i) {
        if (enabled_[i] && effects_[i]) {
            effects_[i]->Draw();
        }
    }
}

void PostEffectManager::PostRenderAll() {
    for (size_t i = 0; i < effects_.size(); ++i) {
        if (enabled_[i] && effects_[i]) {
            effects_[i]->PostRender();
        }
    }
}

void PostEffectManager::SetTimeParams(float time) {
    for (size_t i = 0; i < effects_.size(); ++i) {
        if (effects_[i]) {
            effects_[i]->SetTimeParams(time);
        }
    }
}

void PostEffectManager::PreBarrierAll()
{
    for (size_t i = 0; i < effects_.size(); ++i) {
        if (enabled_[i] && effects_[i]) {
            effects_[i]->TransitionRenderTextureToShaderResource();
        }
	}
}

void PostEffectManager::PostBarrierAll()
{
    for (size_t i = 0; i < effects_.size(); ++i) {
        if (enabled_[i] && effects_[i]) {
            effects_[i]->TransitionRenderTextureToRenderTarget();
        }
	}
}

void PostEffectManager::DrawImGui() {  
    ImGui::Begin("Post Effects");
    for (size_t i = 0; i < effects_.size(); ++i) {
        if (effects_[i]) {
            bool enabled = enabled_[i];
            if (ImGui::Checkbox(effects_[i]->GetName(), &enabled)) {
                // 無効化時はSRVへ
                if (enabled_[i] && !enabled) {
                    effects_[i]->TransitionRenderTextureToShaderResource();
                }
                // 有効化時はRTVへ
                if (!enabled_[i] && enabled) {
                    effects_[i]->TransitionRenderTextureToRenderTarget();
                }
                enabled_[i] = enabled;
            }
        }
    }
    ImGui::End();
}

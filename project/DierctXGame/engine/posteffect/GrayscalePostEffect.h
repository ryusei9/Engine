#pragma once
#include "PostEffectBase.h"

/// <summary>
/// グレースケールポストエフェクト
/// </summary>
class GrayscalePostEffect : public PostEffectBase {
public:
	// 初期化
    void Initialize(DirectXCommon* dxCommon) override;

	// 描画前準備
    void PreRender() override;

	// 描画
    void Draw() override;

	// 描画後処理
    void PostRender() override;

    /*------ゲッター------*/

    // エフェクト名の取得
    const char* GetName() const override { return "Gray"; }

	// outputSRVDescriptorHeapIndex_の取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetOutputSRV() const override;

	/*------セッター------*/
	// 時間パラメータの設定
    void SetTimeParams(float time) override {}

private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
    Microsoft::WRL::ComPtr<IDxcBlob> vsBlob_;
    Microsoft::WRL::ComPtr<IDxcBlob> psBlob_;
    DirectXCommon* dxCommon_ = nullptr;

	// ルートシグネチャの作成
    void CreateRootSignature();

	// パイプラインステートオブジェクトの作成
    void CreatePipelineStateObject();
};
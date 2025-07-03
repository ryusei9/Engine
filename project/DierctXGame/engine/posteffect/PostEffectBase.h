#pragma once
#include "DirectXCommon.h"
#include <wrl.h>

// ポストエフェクト用のベースクラス
class PostEffectBase {
public:
    virtual ~PostEffectBase() = default;

    // 初期化（リソース生成など）
    virtual void Initialize(DirectXCommon* dxCommon);

    // ルートシグネイチャの作成
	virtual void CreateRootSignature() = 0;

    // PSOの作成
	virtual void CreatePipelineStateObject() = 0;

    // 描画前の準備（レンダーターゲット切り替えなど）
    virtual void PreRender() = 0;

    // ポストエフェクト本体の描画
    virtual void Draw() = 0;

    // 描画後の後始末（バリア戻しなど）
    virtual void PostRender() = 0;

    virtual void ViewPortInitialize();

	virtual void ScissorRectInitialize();

    virtual void TransitionRenderTextureToRenderTarget();

	virtual void TransitionRenderTextureToShaderResource();

    virtual void CreateRenderTexture(UINT width, UINT height, DXGI_FORMAT format, const Vector4& clearColor);

    // 名前や種類を返す（切り替えUI用）
    virtual const char* GetName() const = 0;

    virtual void SetTimeParams(float) {};
protected:
	DirectXCommon* dxCommon = nullptr; // DirectX共通処理

	ID3D12GraphicsCommandList* commandList = nullptr; // コマンドリスト

    D3D12_RESOURCE_STATES renderTextureState = D3D12_RESOURCE_STATE_RENDER_TARGET;

    Microsoft::WRL::ComPtr<ID3D12Resource> renderTexture;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap; // レンダーターゲットビュー用ヒープ

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle; // レンダーターゲットビューのハンドル

    const Vector4 kRenderTargetClearValue = { 0.1f,0.25f,0.5f,1.0f };

    D3D12_VIEWPORT viewport{};

    D3D12_RECT scissorRect{};
};
#pragma once
#include "Camera.h"
#include <memory>
#include <wrl.h>
#include <d3d12.h>
#include <dxcapi.h>

class SrvManager;// 前方宣言
class DirectXCommon;// 前方宣言

/// <summary>
/// 3Dオブジェクト共通部
/// </summary>
class Object3dCommon
{
public:
	// シングルトンインスタンスの取得
	static std::shared_ptr<Object3dCommon> GetInstance();

	Object3dCommon() = default;
	~Object3dCommon() = default;
	Object3dCommon(Object3dCommon&) = delete;
	Object3dCommon& operator=(Object3dCommon&) = delete;

	// 初期化
	void Initialize(SrvManager* srvManager);

	// 共通描画設定
	void DrawSettings();

	/*------セッター------*/
	void SetDefaultCamera(Camera* camera) { defaultCamera_ = camera; }

	/*------ゲッター------*/
	//  DirectXCommonの取得
	DirectXCommon* GetDxCommon() const { return dxCommon_; }

	// デフォルトカメラの取得
	Camera* GetDefaultCamera() const { return defaultCamera_; }

private:
	static std::shared_ptr<Object3dCommon> sInstance_;
	
	// ルートシグネチャの初期化
	void RootSignatureInitialize();

	// グラフィックスパイプラインの初期化
	void GraphicsPipelineInitialize();

	DirectXCommon* dxCommon_;

	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_{};

	D3D12_INPUT_ELEMENT_DESC inputElementDescs_[3] = {};

	/// BlendStateの設定
	D3D12_RENDER_TARGET_BLEND_DESC blendDesc_{};

	/// RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc_{};

	// shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;

	/// PixelShader
	// shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};

	// グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_ = nullptr;

	Camera* defaultCamera_ = nullptr;

	// srvManagerからの借りポインタ
	SrvManager* srvManager_ = nullptr;
};


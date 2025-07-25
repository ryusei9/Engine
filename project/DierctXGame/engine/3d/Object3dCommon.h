#pragma once
#include "Camera.h"
#include <memory>
#include <wrl.h>
#include <d3d12.h>
#include <dxcapi.h>



class SrvManager;
class DirectXCommon;
// 3Dオブジェクト共通部
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
	void Initialize();
	// 共通描画設定
	void DrawSettings();

	/// <summary>
	/// セッター
	/// </summary>
	void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	/// <summary>
	/// ゲッター
	/// </summary>
	DirectXCommon* GetDxCommon()const { return dxCommon_; }

	Camera* GetDefaultCamera()const { return defaultCamera; }
private:
	static std::shared_ptr<Object3dCommon> instance;

	
	// ルートシグネチャの初期化
	void RootSignatureInitialize();

	// グラフィックスパイプラインの初期化
	void GraphicsPipelineInitialize();

	DirectXCommon* dxCommon_;

	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	/// BlendStateの設定
	D3D12_RENDER_TARGET_BLEND_DESC blendDesc{};

	/// RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc{};

	// shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob;

	/// PixelShader
	// shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob;

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

	// グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

	Camera* defaultCamera = nullptr;

	// srvManagerからの借りポインタ
	SrvManager* srvManager_ = nullptr;
};


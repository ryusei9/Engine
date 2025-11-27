#include "SpriteCommon.h"
#include <d3d12.h>
#include <cassert>
#include "Logger.h"

//
// SpriteCommon: スプライト描画に必要な共通設定（RootSignature / PSO 等）を管理するシングルトン
// - 初期化は SrvManager を渡して呼ぶ
// - 描画前に DrawSettings() を呼んでルートシグネチャや PSO をコマンドリストに設定する
//

std::shared_ptr<SpriteCommon> SpriteCommon::instance = nullptr;

// シングルトン取得
// 返り値: SpriteCommon の shared_ptr（初回呼出し時にインスタンス生成）
std::shared_ptr<SpriteCommon> SpriteCommon::GetInstance()
{
	if (instance == nullptr) {
		instance = std::make_shared<SpriteCommon>();
	}
	return instance;
}

// 初期化
// 入力: SrvManager* srvManager - テクスチャ SRV を管理するマネージャ
// 副作用: 内部で DirectXCommon を取得してメンバに保持し、グラフィクスパイプラインを構築する
void SpriteCommon::Initialize(SrvManager* srvManager)
{
	// DirectX 共通インスタンスを取得して保存
	dxCommon_ = DirectXCommon::GetInstance();

	// SRV マネージャを保存
	srvManager_ = srvManager;

	// ルートシグネチャ / PSO を構築
	GraphicsPipelineInitialize();
}

// 描画準備をコマンドリストに設定
// - コマンドリストにデスクリプタヒープ、ルートシグネチャ、PSO、プリミティブトポロジをセットする
// - 呼び出しタイミング: スプライト描画直前
void SpriteCommon::DrawSettings()
{
	ID3D12DescriptorHeap* heaps[] = { srvManager_->GetDescriptorHeap() };
	dxCommon_->GetCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);

	// ルートシグネチャをコマンドリストに設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	// パイプラインステートオブジェクトを設定
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
	// プリミティブトポロジを設定（トライアングルリスト）
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// RootSignature の構築
// - CBV / SRV / Sampler のルートパラメータを定義し、シリアライズ→生成する
// - 入力レイアウトの要素配列 (inputElementDescs) をここで設定する
void SpriteCommon::RootSignatureInitialize()
{
	HRESULT hr;

	// ルートシグネチャ記述子を作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// ルートパラメータを用意（CBV x2, DescriptorTable x1）
	D3D12_ROOT_PARAMETER rootParameters[3] = {};

	// SRV 用のディスクリプタレンジ（テクスチャ1枚想定）
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// CBV（ピクセル／頂点それぞれ1個ずつ）をルートパラメータに設定
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	// ディスクリプタテーブル（SRV）をルートパラメータに設定
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

	// シェーダでの可視化範囲を設定
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// CBV のレジスタ番号を設定（b0/b0 を使用）
	rootParameters[0].Descriptor.ShaderRegister = 0;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	// DescriptorTable にディスクリプタレンジを設定
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	// 静的サンプラを設定（1つのバイリニアサンプラ）
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// ルートパラメータ配列をルートシグネチャ記述子にセット
	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	// ルートシグネチャをシリアライズして生成
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	rootSignature = nullptr;
	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	// 頂点入力レイアウトを設定（POSITION, TEXCOORD）
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// ブレンド設定（アルファ混合）
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// ラスタライザ設定
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// シェーダをコンパイルしてバイナリを保存（VS, PS）
	vertexShaderBlob = dxCommon_->CompileShader(L"Resources/shaders/Sprite.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	pixelShaderBlob = dxCommon_->CompileShader(L"Resources/shaders/Sprite.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	// デプスステンシル設定（Depth 有効、書き込み有効、比較関数 LessEqual）
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}

// グラフィックスパイプライン（PSO）を作成
// - RootSignatureInitialize() で構築した情報をもとに PSO を作る
// - 生成された graphicsPipelineState をメンバに保持する
void SpriteCommon::GraphicsPipelineInitialize()
{
	RootSignatureInitialize();
	HRESULT hr;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
}


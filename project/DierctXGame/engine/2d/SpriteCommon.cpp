#include "SpriteCommon.h"
#include <d3d12.h>
#include <cassert>
#include "Logger.h"

namespace {
	// ディスクリプタヒープ配列サイズ
	constexpr uint32_t kDescriptorHeapCount = 1;

	// ブレンド設定用定数
	constexpr BOOL kBlendEnable = TRUE;
	constexpr BOOL kBlendDisable = FALSE;

	// カリングモード
	constexpr D3D12_CULL_MODE kCullModeNone = D3D12_CULL_MODE_NONE;

	// 塗りつぶしモード
	constexpr D3D12_FILL_MODE kFillModeSolid = D3D12_FILL_MODE_SOLID;

	// デプス設定
	constexpr BOOL kDepthEnable = TRUE;
	constexpr D3D12_DEPTH_WRITE_MASK kDepthWriteMaskAll = D3D12_DEPTH_WRITE_MASK_ALL;
	constexpr D3D12_COMPARISON_FUNC kDepthFuncLessEqual = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// セマンティック名
	constexpr const char* kSemanticPosition = "POSITION";
	constexpr const char* kSemanticTexcoord = "TEXCOORD";

	// セマンティックインデックス
	constexpr UINT kSemanticIndex = 0;

	// シェーダファイルパス
	constexpr const wchar_t* kVertexShaderPath = L"Resources/shaders/Sprite.VS.hlsl";
	constexpr const wchar_t* kPixelShaderPath = L"Resources/shaders/Sprite.PS.hlsl";

	// シェーダモデル
	constexpr const wchar_t* kVertexShaderModel = L"vs_6_0";
	constexpr const wchar_t* kPixelShaderModel = L"ps_6_0";
}

std::shared_ptr<SpriteCommon> SpriteCommon::sInstance_ = nullptr;

// シングルトン取得
// 返り値: SpriteCommon の shared_ptr（初回呼出し時にインスタンス生成）
std::shared_ptr<SpriteCommon> SpriteCommon::GetInstance()
{
	if (sInstance_ == nullptr) {
		sInstance_ = std::make_shared<SpriteCommon>();
	}
	return sInstance_;
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

	// グラフィックスパイプラインを構築
	GraphicsPipelineInitialize();
}

// 描画準備をコマンドリストに設定
// - コマンドリストにデスクリプタヒープ、ルートシグネチャ、PSO、プリミティブトポロジをセットする
// - 呼び出しタイミング: スプライト描画直前
void SpriteCommon::DrawSettings()
{
	// ディスクリプタヒープを設定
	ID3D12DescriptorHeap* heaps[] = { srvManager_->GetDescriptorHeap() };
	dxCommon_->GetCommandList()->SetDescriptorHeaps(kDescriptorHeapCount, heaps);

	// ルートシグネチャを設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());

	// パイプラインステートを設定
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState_.Get());

	// プリミティブトポロジを設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// RootSignature の構築
// - CBV / SRV / Sampler のルートパラメータを定義し、シリアライズ→生成する
// - 入力レイアウトの要素配列 (inputElementDescs_) をここで設定する
void SpriteCommon::RootSignatureInitialize()
{
	HRESULT hr;

	// ルートシグネチャ記述子を作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// ルートパラメータを用意
	D3D12_ROOT_PARAMETER rootParameters[SpriteCommonDefaults::kRootParamCount]{};

	// ディスクリプタレンジ（SRV）
	D3D12_DESCRIPTOR_RANGE descriptorRange[SpriteCommonDefaults::kDescRangeCount]{};
	descriptorRange[0].BaseShaderRegister = SpriteCommonDefaults::kSrvBaseRegister;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ルートパラメータ0: CBV（マテリアル）
	rootParameters[SpriteCommonDefaults::kRootParamIndexMaterial].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[SpriteCommonDefaults::kRootParamIndexMaterial].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[SpriteCommonDefaults::kRootParamIndexMaterial].Descriptor.ShaderRegister = SpriteCommonDefaults::kCbvRegister;

	// ルートパラメータ1: CBV（WVP）
	rootParameters[SpriteCommonDefaults::kRootParamIndexWVP].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[SpriteCommonDefaults::kRootParamIndexWVP].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[SpriteCommonDefaults::kRootParamIndexWVP].Descriptor.ShaderRegister = SpriteCommonDefaults::kCbvRegister;

	// ルートパラメータ2: DescriptorTable（テクスチャ）
	rootParameters[SpriteCommonDefaults::kRootParamIndexTexture].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[SpriteCommonDefaults::kRootParamIndexTexture].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[SpriteCommonDefaults::kRootParamIndexTexture].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[SpriteCommonDefaults::kRootParamIndexTexture].DescriptorTable.NumDescriptorRanges = SpriteCommonDefaults::kDescRangeCount;

	// 静的サンプラを設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[SpriteCommonDefaults::kStaticSamplerCount]{};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = SpriteCommonDefaults::kSrvBaseRegister;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = SpriteCommonDefaults::kStaticSamplerCount;

	// ルートパラメータを設定
	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = SpriteCommonDefaults::kRootParamCount;

	// ルートシグネチャをシリアライズ
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	hr = D3D12SerializeRootSignature(
		&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob,
		&errorBlob);

	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	// ルートシグネチャを作成
	hr = dxCommon_->GetDevice()->CreateRootSignature(
		0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));

	// 入力レイアウトを設定
	inputElementDescs_[0].SemanticName = kSemanticPosition;
	inputElementDescs_[0].SemanticIndex = kSemanticIndex;
	inputElementDescs_[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs_[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs_[1].SemanticName = kSemanticTexcoord;
	inputElementDescs_[1].SemanticIndex = kSemanticIndex;
	inputElementDescs_[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs_[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputLayoutDesc_.pInputElementDescs = inputElementDescs_;
	inputLayoutDesc_.NumElements = SpriteCommonDefaults::kInputElementCount;

	// ブレンド設定
	blendDesc_.RenderTarget[0].BlendEnable = kBlendEnable;
	blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc_.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc_.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc_.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc_.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// ラスタライザ設定
	rasterizerDesc_.CullMode = kCullModeNone;
	rasterizerDesc_.FillMode = kFillModeSolid;

	// シェーダをコンパイル
	vertexShaderBlob_ = dxCommon_->CompileShader(kVertexShaderPath, kVertexShaderModel);
	assert(vertexShaderBlob_ != nullptr);

	pixelShaderBlob_ = dxCommon_->CompileShader(kPixelShaderPath, kPixelShaderModel);
	assert(pixelShaderBlob_ != nullptr);

	// デプスステンシル設定
	depthStencilDesc_.DepthEnable = kDepthEnable;
	depthStencilDesc_.DepthWriteMask = kDepthWriteMaskAll;
	depthStencilDesc_.DepthFunc = kDepthFuncLessEqual;
}

// グラフィックスパイプライン（PSO）を作成
// - RootSignatureInitialize() で構築した情報をもとに PSO を作る
// - 生成された graphicsPipelineState_ をメンバに保持する
void SpriteCommon::GraphicsPipelineInitialize()
{
	// ルートシグネチャを初期化
	RootSignatureInitialize();

	// パイプラインステート記述子を作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc_;
	graphicsPipelineStateDesc.BlendState = blendDesc_;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc_;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob_->GetBufferPointer(), vertexShaderBlob_->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob_->GetBufferPointer(), pixelShaderBlob_->GetBufferSize() };
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc_;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	graphicsPipelineStateDesc.NumRenderTargets = SpriteCommonDefaults::kRenderTargetCount;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.SampleDesc.Count = SpriteCommonDefaults::kSampleCount;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// パイプラインステートを作成
	HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(
		&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState_));
	assert(SUCCEEDED(hr));
}


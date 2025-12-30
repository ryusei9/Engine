#include "Object3dCommon.h"
#include <d3d12.h>
#include <cassert>
#include "Logger.h"
#include <SrvManager.h>
#include "DirectXCommon.h"

//
// Object3dCommon
// - シーン内の 3D 描画で共通に使うパイプライン（RootSignature / PSO / 入力レイアウト 等）を初期化・管理するシングルトン。
// - 各 Object3d は描画前に Object3dCommon::GetInstance()->DrawSettings() を呼んで
//   DescriptorHeap / RootSignature / PSO / トポロジの設定を完了した上で描画を行う想定。
// - 注意:
//   - ルートパラメータやレジスタ番号 (b#, t#) はシェーダ側の定義と一致させる必要がある。
//   - 本クラスは DirectX コマンドリスト等の共通準備を行うのみで、各オブジェクト固有の CBV/SRV は描画側で設定する。
//

std::shared_ptr<Object3dCommon> Object3dCommon::sInstance_ = nullptr;

// シングルトン取得
// - 初回呼び出しでインスタンスを生成して返す（アプリケーション全体で1つだけ保持）
std::shared_ptr<Object3dCommon> Object3dCommon::GetInstance()
{
	if (sInstance_ == nullptr) {
		sInstance_ = std::make_shared<Object3dCommon>();
	}
	return sInstance_;
}

// 初期化
// - SrvManager を受け取り内部に保持する。
// - DirectXCommon インスタンスを取得して保存する。
// - グラフィックスパイプライン（RootSignature + PSO）を構築する。
// - 呼び出しはアプリ起動時に一度だけ行う想定。
void Object3dCommon::Initialize(SrvManager* srvManager)
{
	// DirectX 共通インスタンスを取得して保持
	dxCommon_ = DirectXCommon::GetInstance();

	// SRV 管理用マネージャを保存（DescriptorHeap の取得などで使用）
	srvManager_ = srvManager;

	// ルートシグネチャと PSO の構築
	GraphicsPipelineInitialize();
}

// 描画前設定
// - コマンドリストに対して RootSignature / PSO / プリミティブトポロジ を設定する。
// - また DescriptorHeap（SRV 用）をコマンドリストにセットする。
// - 各 Object の Draw 呼び出し前に必ず実行しておくこと。
void Object3dCommon::DrawSettings()
{
	// RootSignature をコマンドリストに設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	// PSO を設定
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState_.Get());
	// プリミティブトポロジを設定（トライアングルリスト）
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// DescriptorHeap (SRV) をコマンドリストに紐づける
	ID3D12DescriptorHeap* descriptorHeaps[] = { srvManager_->GetDescriptorHeap() };
	dxCommon_->GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
}

// RootSignature の構築
// - ルートパラメータの意味（呼び出し側とシェーダが合致していることが重要）:
//   rootParameters[0] : Material CBV (b0)    - Pixel シェーダで参照
//   rootParameters[1] : Transformation CBV (b0) - Vertex シェーダで参照
//   rootParameters[2] : Texture SRV (t0) via DescriptorTable - Pixel シェーダで参照
//   rootParameters[3] : DirectionalLight CBV (b1) - Pixel シェーダで参照
//   rootParameters[4] : Camera CBV (b2) - Pixel シェーダで参照
//   rootParameters[5] : PointLight CBV (b3) - Pixel シェーダで参照
//   rootParameters[6] : SpotLight CBV (b4) - Pixel シェーダで参照
//   rootParameters[7] : EnvironmentMap SRV (t1) via DescriptorTable - Pixel シェーダで参照
//
// - また、静的サンプラ（バイリニア）をルートシグネチャに含めている。
void Object3dCommon::RootSignatureInitialize()
{
	HRESULT hr;

	// ルートシグネチャ記述
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// ルートパラメータ配列（CBV x6 相当 + 2つの DescriptorTable）
	D3D12_ROOT_PARAMETER rootParameters[8] = {};

	// DescriptorRange: テクスチャ用 SRV (t0) と 環境マップ用 SRV (t1)
	D3D12_DESCRIPTOR_RANGE descriptorRange[2] = {};
	descriptorRange[0].BaseShaderRegister = 0; // t0
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descriptorRange[1].BaseShaderRegister = 1; // t1 (環境マップ等)
	descriptorRange[1].NumDescriptors = 1;
	descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// rootParameters[0] : Material CBV (b0) - Pixel
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].Descriptor.ShaderRegister = 0;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// rootParameters[1] : Transformation CBV (b0) - Vertex
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].Descriptor.ShaderRegister = 0;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// rootParameters[2] : Texture SRV Table (t0) - Pixel
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRange[0];
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// rootParameters[3] : DirectionalLight CBV (b1) - Pixel
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].Descriptor.ShaderRegister = 1;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// rootParameters[4] : Camera CBV (b2) - Pixel
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[4].Descriptor.ShaderRegister = 2;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// rootParameters[5] : PointLight CBV (b3) - Pixel
	rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[5].Descriptor.ShaderRegister = 3;
	rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// rootParameters[6] : SpotLight CBV (b4) - Pixel
	rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[6].Descriptor.ShaderRegister = 4;
	rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// rootParameters[7] : EnvironmentMap SRV Table (t1) - Pixel
	rootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[7].DescriptorTable.pDescriptorRanges = &descriptorRange[1];
	rootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// 静的サンプラ（バイリニア）を定義
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

	// ルートパラメータ配列を設定
	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	// シリアライズしてルートシグネチャを生成
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	rootSignature_ = nullptr;
	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));

	// 入力レイアウトを設定（POSITION, TEXCOORD, NORMAL）
	inputElementDescs_[0].SemanticName = "POSITION";
	inputElementDescs_[0].SemanticIndex = 0;
	inputElementDescs_[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs_[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs_[1].SemanticName = "TEXCOORD";
	inputElementDescs_[1].SemanticIndex = 0;
	inputElementDescs_[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs_[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs_[2].SemanticName = "NORMAL";
	inputElementDescs_[2].SemanticIndex = 0;
	inputElementDescs_[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs_[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputLayoutDesc_.pInputElementDescs = inputElementDescs_;
	inputLayoutDesc_.NumElements = _countof(inputElementDescs_);

	// ブレンド設定（デフォルト: ブレンド無効）
	blendDesc_.BlendEnable = false;
	blendDesc_.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// ラスタライザ設定（裏面カリングなし、塗りつぶし）
	rasterizerDesc_.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc_.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc_.FrontCounterClockwise = FALSE;	 // 時計回りを表面とする

	// シェーダをコンパイルしてバイナリを保持
	vertexShaderBlob_ = dxCommon_->CompileShader(L"Resources/shaders/Object3d.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob_ != nullptr);

	pixelShaderBlob_ = dxCommon_->CompileShader(L"Resources/shaders/Object3d.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob_ != nullptr);

	// デプスステンシル設定（Depth 有効、書き込み有効、比較関数 LessEqual）
	depthStencilDesc_.DepthEnable = true;
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}

// PSO の生成
// - RootSignatureInitialize() で設定した要素を元に D3D12_GRAPHICS_PIPELINE_STATE_DESC を構築して
//   CreateGraphicsPipelineState を呼び、graphicsPipelineState_ を生成する。
void Object3dCommon::GraphicsPipelineInitialize()
{
	RootSignatureInitialize();
	HRESULT hr;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc_;
	graphicsPipelineStateDesc.BlendState.RenderTarget[0] = blendDesc_;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc_;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob_->GetBufferPointer(), vertexShaderBlob_->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob_->GetBufferPointer(), pixelShaderBlob_->GetBufferSize() };
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc_;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState_));
	assert(SUCCEEDED(hr));
}

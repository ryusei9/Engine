#include "Object3dCommon.h"
#include <d3d12.h>
#include <cassert>
#include "Logger.h"

std::shared_ptr<Object3dCommon> Object3dCommon::instance = nullptr;

std::shared_ptr<Object3dCommon> Object3dCommon::GetInstance()
{
	if (instance == nullptr) {
		instance = std::make_shared<Object3dCommon>();
	}
	return instance;
}

void Object3dCommon::Initialize()
{
	// 引数で受け取ってメンバ変数に記録する
	dxCommon_ = DirectXCommon::GetInstance();

	GraphicsPipelineInitialize();
}

void Object3dCommon::DrawSettings()
{
	// RootSignatureを設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	// PSOを設定
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考える
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Object3dCommon::RootSignatureInitialize()
{
	HRESULT hr;
	/// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// Rootparameter作成。複数設定できるので配列
	D3D12_ROOT_PARAMETER rootParameters[8] = {};

	D3D12_DESCRIPTOR_RANGE descriptorRange[2] = {};
	// 0から始まる
	descriptorRange[0].BaseShaderRegister = 0;
	// 数は1つ
	descriptorRange[0].NumDescriptors = 1;
	// SRVを使う
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	// Offsetを自動計算
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// 環境マップ用
	descriptorRange[1].BaseShaderRegister = 1; // t1
	descriptorRange[1].NumDescriptors = 1;
	descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	/*------マテリアル用------*/
	// CBVを使う
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;// b0のbと一致
	// レジスタ番号0とバインド
	rootParameters[0].Descriptor.ShaderRegister = 0;// b0の0と一致
	// PixelShaderを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	/*------transformationMatrix用------*/
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	// vertexShaderで使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	/*------テクスチャ用------*/
	// DescriptorTableを使う
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	// PixelShaderで使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	// Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRange[0];
	// Tableで利用する数
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

	/*------平行光源用------*/
	// CBVを使う
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	// レジスタ番号1を使う
	rootParameters[3].Descriptor.ShaderRegister = 1;

	/*------カメラ用------*/
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // 新しいCBVを追加
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // 新しいCBVの可視性を設定
	// レジスタ番号2を使う
	rootParameters[4].Descriptor.ShaderRegister = 2; // 新しいCBVのレジスタ番号を設定

	/*------ポイントライト用------*/
	rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // 新しいCBVを追加
	rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // 新しいCBVの可視性を設定
	// レジスタ番号2を使う
	rootParameters[5].Descriptor.ShaderRegister = 3; // 新しいCBVのレジスタ番号を設定

	/*------スポットライト用------*/
	rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // 新しいCBVを追加
	rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // 新しいCBVの可視性を設定
	// レジスタ番号2を使う
	rootParameters[6].Descriptor.ShaderRegister = 4; // 新しいCBVのレジスタ番号を設定

	///*------環境マップ用------*/
	rootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	// PixelShaderで使う
	rootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	// Tableの中身の配列を指定
	rootParameters[7].DescriptorTable.pDescriptorRanges = &descriptorRange[1];
	// Tableで利用する数
	rootParameters[7].DescriptorTable.NumDescriptorRanges = 1;

	//////////////////////////
	// Samplerの設定
	//////////////////////////
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	// バイリニアフィルタ
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	// 0~1の範囲外をリピート
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	// 比較しない
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	// ありったけのMipmapを使う
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	// レジスタ番号0を使う
	staticSamplers[0].ShaderRegister = 0;
	// PixelShaderで使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// ルートパラメータ配列へのポインタ
	descriptionRootSignature.pParameters = rootParameters;
	// 配列の長さ
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	// バイナリを元に生成
	rootSignature = nullptr;
	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
	/// InputLayout

	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	/*D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};*/
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	/*/// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};*/
	// ブレンドするかしないか
	blendDesc.BlendEnable = false;
	// すべての色要素を書き込む
	blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	/*/// RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc{};*/
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.FrontCounterClockwise = FALSE;	 // 時計回りの面を表面とする（カリング方向の設定）

	/// VertexShader
	// shaderをコンパイルする
	vertexShaderBlob = dxCommon_->CompileShader(L"Resources/shaders/Object3d.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	/// PixelShader
	// shaderをコンパイルする
	pixelShaderBlob = dxCommon_->CompileShader(L"Resources/shaders/Object3d.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	// DepthStencilStateの設定
	//D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	// 書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}

void Object3dCommon::GraphicsPipelineInitialize()
{
	RootSignatureInitialize();
	HRESULT hr;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	// RootSignature
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
	// InputLayout
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	// Blendstate
	graphicsPipelineStateDesc.BlendState.RenderTarget[0] = blendDesc;
	// RasterizerState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	// VertexShader
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
	// PixelShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
	// DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// 利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// どのように画面に打ち込むのかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// 実際に生成
	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
}

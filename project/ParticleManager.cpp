#include "ParticleManager.h"
#include <Logger.h>

using namespace Logger;
void ParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager)
{
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
}

void ParticleManager::CreateRootSignature()
{
	/*------RootSignature作成------*/
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// テクスチャのディスクリプタヒープを作成する
	D3D12_DESCRIPTOR_RANGE rangeTexture[1]{};
	rangeTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeTexture[0].NumDescriptors = 1;
	rangeTexture[0].BaseShaderRegister = 0;
	rangeTexture[0].RegisterSpace = 0;

	D3D12_ROOT_PARAMETER rootParameters[3]{};
	// CBVを使う
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	// レジスタ番号0とバインド
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 0;
	// pixelShaderで使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// DescriptorTableを使う
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	// vertexShaderで使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	// レジスタ番号0とバインド
	rootParameters[1].Descriptor.ShaderRegister = 0;
	rootParameters[1].DescriptorTable.pDescriptorRanges = rangeTexture;
	// Tableで利用する数
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(rangeTexture);

	// DescriptorTableを使う
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	// PixelShaderで使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	// Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.pDescriptorRanges = rangeTexture;
	// Tableで利用する数
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(rangeTexture);


	descriptionRootSignature.NumParameters = _countof(rootParameters);
	descriptionRootSignature.pParameters = rootParameters;
	

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

	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);
	descriptionRootSignature.pStaticSamplers = staticSamplers;

	// シリアライズされたデータを格納するためのバッファ
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	// エラーメッセージを格納するためのバッファ
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	// バイナリを元に生成
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
}

#include "Skybox.h"
#include <cassert>
#include <cstring>
#include <DirectXCommon.h>
#include <TextureManager.h>
#include <MakeAffineMatrix.h>
#include <Object3dCommon.h>
#include <MakeIdentity4x4.h>
#include <imgui.h>

void Skybox::Initialize(const std::string& texturePath)
{
	filePath_ = texturePath;
	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();

	CreateVertexBuffer();
	CreateTexture(filePath_);
	CreateMaterialResource();
	CreateIndexBuffer();

	// RootSignatureとPSO初期化
	CreateRootSignature();
	CreatePipelineState();

	worldTransform_.Initialize();

	worldTransform_.scale_ = { 50.0f, 50.0f, 50.0f }; // スカイボックスの大きさを調整
	worldTransform_.rotate_ = { 0.0f, 0.0f, 0.0f }; // 初期回転
	worldTransform_.translate_ = { 0.0f, 0.0f, -10.0f }; // 初期位置
}

void Skybox::CreateVertexBuffer()
{
	// 8頂点（各面4頂点ずつ、インデックスで面を作る場合は24頂点でもOK）
	// ここでは各面ごとに4頂点ずつ定義（内側向き）
	vertices_ = {
		// 右面（+X）
		{ {  1.0f,  1.0f,  1.0f, 1.0f }, {  1.0f,  1.0f,  1.0f } },
		{ {  1.0f,  1.0f, -1.0f, 1.0f }, {  1.0f,  1.0f, -1.0f } },
		{ {  1.0f, -1.0f,  1.0f, 1.0f }, {  1.0f, -1.0f,  1.0f } },
		{ {  1.0f, -1.0f, -1.0f, 1.0f }, {  1.0f, -1.0f, -1.0f } },


		{ { -1.0f,  1.0f, -1.0f, 1.0f }, { -1.0f,  1.0f, -1.0f } },
		{ { -1.0f,  1.0f,  1.0f, 1.0f }, { -1.0f,  1.0f,  1.0f } },
		{ { -1.0f, -1.0f, -1.0f, 1.0f }, { -1.0f, -1.0f, -1.0f } },
		{ { -1.0f, -1.0f,  1.0f, 1.0f }, { -1.0f, -1.0f,  1.0f } },


		{ { -1.0f,  1.0f,  1.0f, 1.0f }, { -1.0f,  1.0f,  1.0f } },
		{ {  1.0f,  1.0f,  1.0f, 1.0f }, {  1.0f,  1.0f,  1.0f } },
		{ { -1.0f, -1.0f,  1.0f, 1.0f }, { -1.0f, -1.0f,  1.0f } },
		{ {  1.0f, -1.0f,  1.0f, 1.0f }, {  1.0f, -1.0f,  1.0f } },


		{ { -1.0f,  1.0f, -1.0f, 1.0f }, { -1.0f,  1.0f, -1.0f } },
		{ {  1.0f,  1.0f, -1.0f, 1.0f }, {  1.0f,  1.0f, -1.0f } },
		{ { -1.0f, -1.0f, -1.0f, 1.0f }, { -1.0f, -1.0f, -1.0f } },
		{ {  1.0f, -1.0f, -1.0f, 1.0f }, {  1.0f, -1.0f, -1.0f } },


		{ { -1.0f,  1.0f, -1.0f, 1.0f }, { -1.0f,  1.0f, -1.0f } },
		{ {  1.0f,  1.0f, -1.0f, 1.0f }, {  1.0f,  1.0f, -1.0f } },
		{ { -1.0f,  1.0f,  1.0f, 1.0f }, { -1.0f,  1.0f,  1.0f } },
		{ {  1.0f,  1.0f,  1.0f, 1.0f }, {  1.0f,  1.0f,  1.0f } },


		{ { -1.0f, -1.0f,  1.0f, 1.0f }, { -1.0f, -1.0f,  1.0f } },
		{ {  1.0f, -1.0f,  1.0f, 1.0f }, {  1.0f, -1.0f,  1.0f } },
		{ { -1.0f, -1.0f, -1.0f, 1.0f }, { -1.0f, -1.0f, -1.0f } },
		{ {  1.0f, -1.0f, -1.0f, 1.0f }, {  1.0f, -1.0f, -1.0f } }
	};

	const UINT size = static_cast<UINT>(sizeof(Vertex) * vertices_.size());

	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = size;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	HRESULT hr = DirectXCommon::GetInstance()->GetDevice()->CreateCommittedResource(
		&heapProp, D3D12_HEAP_FLAG_NONE, &resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&vertexBuffer_));
	assert(SUCCEEDED(hr));

	void* mapped = nullptr;
	vertexBuffer_->Map(0, nullptr, &mapped);
	std::memcpy(mapped, vertices_.data(), size);
	vertexBuffer_->Unmap(0, nullptr);

	vbView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vbView_.SizeInBytes = size;
	vbView_.StrideInBytes = sizeof(Vertex);
}

void Skybox::CreateTexture(const std::string& texturePath)
{
	// TextureManager経由でSRVを取得
	TextureManager* texMgr = TextureManager::GetInstance().get();
	texMgr->LoadTexture(texturePath);
	textureHandle_ = texMgr->GetSrvHandleGPU(texturePath);


}

void Skybox::SetCamera(Camera* camera)
{
	camera_ = camera;
}

void Skybox::CreateMaterialResource()
{
	auto* dxCommon = DirectXCommon::GetInstance();
	materialResource_ = dxCommon->CreateBufferResource(sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->uvTransform = MakeIdentity4x4::MakeIdentity4x4();
}

void Skybox::Update()
{
	worldTransform_.Update();
}


void Skybox::CreateRootSignature()
{
	HRESULT hr;
	D3D12_ROOT_SIGNATURE_DESC desc{};
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// RootParameter
	D3D12_ROOT_PARAMETER rootParameters[3] = {};

	// マテリアル用
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].Descriptor.ShaderRegister = 0;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// SRV (t0, PS)
	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.NumDescriptors = 1;
	descriptorRange.BaseShaderRegister = 0;
	descriptorRange.RegisterSpace = 0;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	/*------transformationMatrix用------*/
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	// vertexShaderで使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	desc.NumParameters = _countof(rootParameters);
	desc.pParameters = rootParameters;

	// Sampler
	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	desc.NumStaticSamplers = 1;
	desc.pStaticSamplers = &samplerDesc;

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	assert(SUCCEEDED(hr));
	hr = DirectXCommon::GetInstance()->GetDevice()->CreateRootSignature(
		0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
}

void Skybox::CreatePipelineState()
{
	HRESULT hr;
	// InputLayout
	inputElementDescs_[0].SemanticName = "POSITION";
	inputElementDescs_[0].SemanticIndex = 0;
	inputElementDescs_[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs_[0].InputSlot = 0;
	inputElementDescs_[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs_[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	inputElementDescs_[0].InstanceDataStepRate = 0;

	inputElementDescs_[1].SemanticName = "TEXCOORD";
	inputElementDescs_[1].SemanticIndex = 0;
	inputElementDescs_[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs_[1].InputSlot = 0;
	inputElementDescs_[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs_[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	inputElementDescs_[1].InstanceDataStepRate = 0;

	inputLayoutDesc_.pInputElementDescs = inputElementDescs_;
	inputLayoutDesc_.NumElements = _countof(inputElementDescs_);

	// Blend
	blendDesc_.BlendEnable = FALSE;
	blendDesc_.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// Rasterizer
	rasterizerDesc_.CullMode = D3D12_CULL_MODE_FRONT;
	rasterizerDesc_.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc_.FrontCounterClockwise = FALSE;

	// DepthStencil
	depthStencilDesc_.DepthEnable = TRUE;
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// シェーダーのコンパイル
	auto* dxCommon = DirectXCommon::GetInstance();
	auto vsBlob = dxCommon->CompileShader(L"resources/shaders/Skybox.VS.hlsl", L"vs_6_0");
	auto psBlob = dxCommon->CompileShader(L"resources/shaders/Skybox.PS.hlsl", L"ps_6_0");
	assert(vsBlob && psBlob);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.pRootSignature = rootSignature_.Get();
	psoDesc.InputLayout = inputLayoutDesc_;
	psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
	psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
	psoDesc.BlendState.RenderTarget[0] = blendDesc_;
	psoDesc.RasterizerState = rasterizerDesc_;
	psoDesc.DepthStencilState = depthStencilDesc_;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_));
	assert(SUCCEEDED(hr));
}

void Skybox::CreateIndexBuffer()
{
	// インデックス用のリソースを作成
	indexResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t) * kNumIndex);

	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * kNumIndex;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));

	// 右
	indexData_[0] = 2; indexData_[1] = 1; indexData_[2] = 0;
	indexData_[3] = 3; indexData_[4] = 1; indexData_[5] = 2;

	// 左
	indexData_[6] = 6; indexData_[7] = 5; indexData_[8] = 4;
	indexData_[9] = 7; indexData_[10] = 5; indexData_[11] = 6;

	// 前面（+Z）
	indexData_[12] = 10; indexData_[13] = 9; indexData_[14] = 8;
	indexData_[15] = 11; indexData_[16] = 9; indexData_[17] = 10;

	// 後面（-Z）
	indexData_[18] = 12; indexData_[19] = 13; indexData_[20] = 14;
	indexData_[21] = 14; indexData_[22] = 13; indexData_[23] = 15;

	// 上面（+Y）
	indexData_[24] = 17; indexData_[25] = 16; indexData_[26] = 18;
	indexData_[27] = 17; indexData_[28] = 18; indexData_[29] = 19;

	// 下面（-Y）
	indexData_[30] = 21; indexData_[31] = 20; indexData_[32] = 22;
	indexData_[33] = 21; indexData_[34] = 22; indexData_[35] = 23;

	memcpy(indexData_, &indexData_[0], sizeof(uint32_t) * kNumIndex);

}

void Skybox::Draw()
{
	auto* cmdList = DirectXCommon::GetInstance()->GetCommandList();

	// RootSignatureとPSOをセット
	cmdList->SetGraphicsRootSignature(rootSignature_.Get());
	cmdList->SetPipelineState(pipelineState_.Get());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// VBVセット
	cmdList->IASetVertexBuffers(0, 1, &vbView_);
	cmdList->IASetIndexBuffer(&indexBufferView_);

	// CBVとSRVセット
	cmdList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	worldTransform_.SetPipeline();
	cmdList->SetGraphicsRootDescriptorTable(2, textureHandle_);

	cmdList->DrawIndexedInstanced(kNumIndex, 1, 0, 0, 0);
}

void Skybox::DrawImGui()
{
	ImGui::Begin("Skybox");
	ImGui::ColorEdit4("Color", &materialData_->color.x);
	ImGui::SliderFloat3("Position", &worldTransform_.translate_.x, -10.0f, 10.0f);
	ImGui::SliderAngle("Rotation.x", &worldTransform_.rotate_.x);
	ImGui::SliderAngle("Rotation.y", &worldTransform_.rotate_.y);
	ImGui::SliderAngle("Rotation.z", &worldTransform_.rotate_.z);
	ImGui::SliderFloat3("Scale", &worldTransform_.scale_.x, 0.1f, 50.0f);
	ImGui::End();
}

void Skybox::DrawSettings() {
	auto* cmdList = DirectXCommon::GetInstance()->GetCommandList();

	cmdList->SetGraphicsRootSignature(rootSignature_.Get());
	cmdList->SetPipelineState(pipelineState_.Get());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

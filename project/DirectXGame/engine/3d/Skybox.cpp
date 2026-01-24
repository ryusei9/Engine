#include "Skybox.h"
#include <cassert>
#include <cstring>
#include <DirectXCommon.h>
#include <TextureManager.h>
#include <MakeAffineMatrix.h>
#include <Object3dCommon.h>
#include <MakeIdentity4x4.h>
#include <imgui.h>

//
// Skybox
// - シーン背景に描画するキューブ型スカイボックスを扱うクラス。
// - 目的：遠景を埋めるための立方体マップ風テクスチャを描画し、カメラ位置に固定して視界いっぱいに見せる。
// - 実装方針：内向き法線のキューブを大きくスケールして描画。頂点・インデックスは CPU 側で作成し Upload ヒープに保持する。
// - 注意点：
//   - スカイボックスはカメラの位置に追従して移動させ、回転だけ反映する（通常はワールド平行移動を無視する）
//   - 現在の実装は単一テクスチャ（立方体マップではなく通常のテクスチャを各面に割り当てる想定）を想定している
//   - リソースは Upload ヒープに置く簡易実装。大規模な最適化やストリーミングは未対応。
//

using namespace SkyboxConstants;

void Skybox::Initialize(const std::string& texturePath)
{
	// 入力:
	// - texturePath: スカイボックス用テクスチャのファイルパス（TextureManager が扱える形式）
	// 副作用:
	// - 頂点バッファ / インデックスバッファ / マテリアル CBV を作成して初期化する
	// - RootSignature / PSO を生成する
	// - worldTransform_ を初期化してデフォルトのスケール・位置を設定する
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

	// スカイボックスは大きくスケールして遠景を覆う（値は調整可能）
	worldTransform_.scale_ = { kDefaultScale, kDefaultScale, kDefaultScale };
	worldTransform_.rotate_ = { kDefaultRotation, kDefaultRotation, kDefaultRotation };
	worldTransform_.translate_ = { 0.0f, 0.0f, kDefaultTranslateZ };
}

void Skybox::CreateVertexBuffer()
{
	// 頂点構成:
	// - 各面を 4 頂点で定義（合計 24 頂点、内側向きに配置）
	// - Vertex に position と texcoord を保持（シンプルな Skybox 用）
	// 実装メモ:
	// - Upload ヒープにバッファを確保して CPU 側データを memcpy する簡易実装
	InitializeVertexData();

	const uint32_t size = static_cast<uint32_t>(sizeof(Vertex) * vertices_.size());

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
	// TextureManager経由で SRV を取得して保持する
	TextureManager* texMgr = TextureManager::GetInstance().get();
	texMgr->LoadTexture(texturePath);
	textureHandle_ = texMgr->GetSrvHandleGPU(texturePath);
}

void Skybox::SetCamera(Camera* camera)
{
	// 描画に利用するカメラを差し替える（外部からカメラを渡せる）
	camera_ = camera;
}

void Skybox::CreateMaterialResource()
{
	// マテリアル用 CBV を生成してデフォルト値を設定
	auto* dxCommon = DirectXCommon::GetInstance();
	materialResource_ = dxCommon->CreateBufferResource(sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	materialData_->color = { kDefaultColorR, kDefaultColorG, kDefaultColorB, kDefaultColorA };
	materialData_->uvTransform = MakeIdentity4x4::MakeIdentity4x4();
}

void Skybox::Update()
{
	// worldTransform_ の内部状態を更新（必要に応じて position/rotation/scale を変更した後に呼ぶ）
	worldTransform_.Update();
}

void Skybox::CreateRootSignature()
{
	// ルートシグネチャ構築
	// - rootParameters[0] : Material CBV (b0) - Pixel
	// - rootParameters[1] : Transformation CBV (b0) - Vertex
	// - rootParameters[2] : Texture SRV Table (t0) - Pixel
	HRESULT hr;
	D3D12_ROOT_SIGNATURE_DESC desc{};
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_ROOT_PARAMETER rootParameters[kRootParameterCount] = {};
	D3D12_DESCRIPTOR_RANGE descriptorRange{};

	SetupRootParameters(rootParameters, &descriptorRange);

	desc.NumParameters = kRootParameterCount;
	desc.pParameters = rootParameters;

	// 静的サンプラ設定（バイリニア）
	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	SetupStaticSampler(&samplerDesc);
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
	// PSO の生成：
	// - 入力レイアウトは POSITION/TEXCOORD を使用
	// - Rasterizer のカリングは front-face をカリング（内側から見るため front をカリング）
	// - DepthWrite は無効にして深度テストは <= で評価（スカイボックスが最奥に描かれる想定）
	HRESULT hr;
	
	SetupInputLayout();
	SetupBlendState();
	SetupRasterizerState();
	SetupDepthStencilState();

	// シェーダーのコンパイル（外部の DirectXCommon ラッパを利用）
	auto* dxCommon = DirectXCommon::GetInstance();
	auto vsBlob = dxCommon->CompileShader(kVertexShaderPath, kVertexShaderProfile);
	auto psBlob = dxCommon->CompileShader(kPixelShaderPath, kPixelShaderProfile);
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
	// インデックスバッファを Upload ヒープに作成し、各面ごとの三角形インデックスを書き込む
	indexResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t) * kNumIndices);

	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * kNumIndices;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
	InitializeIndexData();
}

void Skybox::Draw()
{
	// 描画コマンド発行:
	// - RootSignature/PSO をセットし、VB/IB をバインド、CBV/SRV をルートに配置して DrawIndexedInstanced を呼ぶ
	auto* cmdList = DirectXCommon::GetInstance()->GetCommandList();

	// RootSignatureとPSOをセット
	cmdList->SetGraphicsRootSignature(rootSignature_.Get());
	cmdList->SetPipelineState(pipelineState_.Get());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// VBVセット
	cmdList->IASetVertexBuffers(0, 1, &vbView_);
	cmdList->IASetIndexBuffer(&indexBufferView_);

	// マテリアル CBV と transformation（world）をセット
	cmdList->SetGraphicsRootConstantBufferView(kRootParameterIndexMaterial, materialResource_->GetGPUVirtualAddress());
	worldTransform_.SetPipeline();

	// テクスチャ SRV (t0)
	cmdList->SetGraphicsRootDescriptorTable(kRootParameterIndexTexture, textureHandle_);

	// インデックス描画
	cmdList->DrawIndexedInstanced(kNumIndices, 1, 0, 0, 0);
}

void Skybox::DrawImGui()
{
#ifdef USE_IMGUI
	// デバッグ用 UI: 色・位置・回転・スケールを即時変更できる
	ImGui::Begin("Skybox");
	ImGui::ColorEdit4("Color", &materialData_->color.x);
	ImGui::SliderFloat3("Position", &worldTransform_.translate_.x, -10.0f, 10.0f);
	ImGui::SliderAngle("Rotation.x", &worldTransform_.rotate_.x);
	ImGui::SliderAngle("Rotation.y", &worldTransform_.rotate_.y);
	ImGui::SliderAngle("Rotation.z", &worldTransform_.rotate_.z);
	ImGui::SliderFloat3("Scale", &worldTransform_.scale_.x, 0.1f, 50.0f);
	ImGui::End();
#endif
}

void Skybox::DrawSettings() {
	// 描画前に PSO/RootSignature/Topology をコマンドリストに設定する補助関数
	auto* cmdList = DirectXCommon::GetInstance()->GetCommandList();

	cmdList->SetGraphicsRootSignature(rootSignature_.Get());
	cmdList->SetPipelineState(pipelineState_.Get());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// ===== ヘルパー関数 =====

void Skybox::SetupInputLayout()
{
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
	inputLayoutDesc_.NumElements = kInputElementCount;
}

void Skybox::SetupBlendState()
{
	// Blend（無効）
	blendDesc_.BlendEnable = FALSE;
	blendDesc_.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
}

void Skybox::SetupRasterizerState()
{
	// Rasterizer : 内側を描くためにフロントカリング（Front = カリング）
	rasterizerDesc_.CullMode = D3D12_CULL_MODE_FRONT;
	rasterizerDesc_.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc_.FrontCounterClockwise = FALSE;
}

void Skybox::SetupDepthStencilState()
{
	// DepthStencil: 書き込み無効（スカイボックスは深度を上書きしない）
	depthStencilDesc_.DepthEnable = TRUE;
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}

void Skybox::SetupRootParameters(D3D12_ROOT_PARAMETER* rootParameters, D3D12_DESCRIPTOR_RANGE* descriptorRange)
{
	// マテリアル用 CBV
	rootParameters[kRootParameterIndexMaterial].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[kRootParameterIndexMaterial].Descriptor.ShaderRegister = kMaterialRegister;
	rootParameters[kRootParameterIndexMaterial].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// SRV (t0, PS)
	descriptorRange->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange->NumDescriptors = 1;
	descriptorRange->BaseShaderRegister = kTextureRegister;
	descriptorRange->RegisterSpace = 0;
	descriptorRange->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// transformationMatrix 用 CBV（頂点シェーダ用）
	rootParameters[kRootParameterIndexTransformation].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[kRootParameterIndexTransformation].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[kRootParameterIndexTransformation].Descriptor.ShaderRegister = kTransformationRegister;

	// テクスチャ用 DescriptorTable
	rootParameters[kRootParameterIndexTexture].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[kRootParameterIndexTexture].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[kRootParameterIndexTexture].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[kRootParameterIndexTexture].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
}

void Skybox::SetupStaticSampler(D3D12_STATIC_SAMPLER_DESC* samplerDesc)
{
	samplerDesc->Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc->AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc->AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc->AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc->ShaderRegister = kSamplerRegister;
	samplerDesc->ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
}

void Skybox::InitializeVertexData()
{
	vertices_ = {
		// 右面（+X）
		{ {  1.0f,  1.0f,  1.0f, 1.0f }, {  1.0f,  1.0f,  1.0f } },
		{ {  1.0f,  1.0f, -1.0f, 1.0f }, {  1.0f,  1.0f, -1.0f } },
		{ {  1.0f, -1.0f,  1.0f, 1.0f }, {  1.0f, -1.0f,  1.0f } },
		{ {  1.0f, -1.0f, -1.0f, 1.0f }, {  1.0f, -1.0f, -1.0f } },

		// 左面（-X）
		{ { -1.0f,  1.0f, -1.0f, 1.0f }, { -1.0f,  1.0f, -1.0f } },
		{ { -1.0f,  1.0f,  1.0f, 1.0f }, { -1.0f,  1.0f,  1.0f } },
		{ { -1.0f, -1.0f, -1.0f, 1.0f }, { -1.0f, -1.0f, -1.0f } },
		{ { -1.0f, -1.0f,  1.0f, 1.0f }, { -1.0f, -1.0f,  1.0f } },

		// 前面 (+Z)
		{ { -1.0f,  1.0f,  1.0f, 1.0f }, { -1.0f,  1.0f,  1.0f } },
		{ {  1.0f,  1.0f,  1.0f, 1.0f }, {  1.0f,  1.0f,  1.0f } },
		{ { -1.0f, -1.0f,  1.0f, 1.0f }, { -1.0f, -1.0f,  1.0f } },
		{ {  1.0f, -1.0f,  1.0f, 1.0f }, {  1.0f, -1.0f,  1.0f } },

		// 後面 (-Z)
		{ { -1.0f,  1.0f, -1.0f, 1.0f }, { -1.0f,  1.0f, -1.0f } },
		{ {  1.0f,  1.0f, -1.0f, 1.0f }, {  1.0f,  1.0f, -1.0f } },
		{ { -1.0f, -1.0f, -1.0f, 1.0f }, { -1.0f, -1.0f, -1.0f } },
		{ {  1.0f, -1.0f, -1.0f, 1.0f }, {  1.0f, -1.0f, -1.0f } },

		// 上面 (+Y)
		{ { -1.0f,  1.0f, -1.0f, 1.0f }, { -1.0f,  1.0f, -1.0f } },
		{ {  1.0f,  1.0f, -1.0f, 1.0f }, {  1.0f,  1.0f, -1.0f } },
		{ { -1.0f,  1.0f,  1.0f, 1.0f }, { -1.0f,  1.0f,  1.0f } },
		{ {  1.0f,  1.0f,  1.0f, 1.0f }, {  1.0f,  1.0f,  1.0f } },

		// 下面 (-Y)
		{ { -1.0f, -1.0f,  1.0f, 1.0f }, { -1.0f, -1.0f,  1.0f } },
		{ {  1.0f, -1.0f,  1.0f, 1.0f }, {  1.0f, -1.0f,  1.0f } },
		{ { -1.0f, -1.0f, -1.0f, 1.0f }, { -1.0f, -1.0f, -1.0f } },
		{ {  1.0f, -1.0f, -1.0f, 1.0f }, {  1.0f, -1.0f, -1.0f } }
	};
}

void Skybox::InitializeIndexData()
{
	// 各面のインデックス（右、左、前、後、上、下）
	indexData_[0] = 2; indexData_[1] = 1; indexData_[2] = 0;
	indexData_[3] = 3; indexData_[4] = 1; indexData_[5] = 2;

	indexData_[6] = 6; indexData_[7] = 5; indexData_[8] = 4;
	indexData_[9] = 7; indexData_[10] = 5; indexData_[11] = 6;

	indexData_[12] = 10; indexData_[13] = 9; indexData_[14] = 8;
	indexData_[15] = 11; indexData_[16] = 9; indexData_[17] = 10;

	indexData_[18] = 12; indexData_[19] = 13; indexData_[20] = 14;
	indexData_[21] = 14; indexData_[22] = 13; indexData_[23] = 15;

	indexData_[24] = 17; indexData_[25] = 16; indexData_[26] = 18;
	indexData_[27] = 17; indexData_[28] = 18; indexData_[29] = 19;

	indexData_[30] = 21; indexData_[31] = 20; indexData_[32] = 22;
	indexData_[33] = 21; indexData_[34] = 22; indexData_[35] = 23;
}

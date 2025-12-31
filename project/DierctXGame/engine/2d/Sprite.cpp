#include "Sprite.h"
#include "SpriteCommon.h"
#include "Transform.h"
#include "MakeAffineMatrix.h"
#include "MakePerspectiveFovMatrix.h"
#include "Multiply.h"
#include "MakeOrthographicMatrix.h"
#include <cassert>

namespace {
	// 頂点インデックスの定義（ファイル内限定）
	constexpr uint32_t kTriangle1Index0 = 0;
	constexpr uint32_t kTriangle1Index1 = 1;
	constexpr uint32_t kTriangle1Index2 = 2;
	constexpr uint32_t kTriangle2Index0 = 1;
	constexpr uint32_t kTriangle2Index1 = 3;
	constexpr uint32_t kTriangle2Index2 = 2;

	// アンカーポイント計算用定数
	constexpr float kAnchorLeft = 0.0f;
	constexpr float kAnchorRight = 1.0f;
	constexpr float kAnchorTop = 0.0f;
	constexpr float kAnchorBottom = 1.0f;

	// 法線ベクトル（スプライトは常に-Z方向）
	constexpr Vector3 kSpriteNormal = { 0.0f, 0.0f, -1.0f };

	// スプライトのZ座標（常に0）
	constexpr float kSpriteDepth = 0.0f;

	// 同次座標のW成分
	constexpr float kHomogeneousW = 1.0f;

	// スプライトのZ軸スケール（常に1）
	constexpr float kSpriteScaleZ = 1.0f;
}

void Sprite::Initialize(DirectXCommon* dxCommon, std::string textureFilePath)
{
	dxCommon_ = dxCommon;
	filePath_ = textureFilePath;
	
	// 頂点データの作成
	CreateVertexData();

	// Materialデータの作成
	CreateMaterialData();

	// WVPデータの作成
	CreateWVPData();

	// テクスチャサイズをイメージに合わせる
	AdjustTextureSize();
}

void Sprite::Update()
{
	// アンカーポイントを考慮した頂点座標の計算
	float left = kAnchorLeft - anchorPoint_.x;
	float right = kAnchorRight - anchorPoint_.x;
	float top = kAnchorTop - anchorPoint_.y;
	float bottom = kAnchorBottom - anchorPoint_.y;

	// テクスチャメタデータの取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(filePath_);
	
	// テクスチャ座標の正規化
	float tex_left = textureLeftTop_.x / static_cast<float>(metadata.width);
	float tex_right = (textureLeftTop_.x + textureSize_.x) / static_cast<float>(metadata.width);
	float tex_top = textureLeftTop_.y / static_cast<float>(metadata.height);
	float tex_bottom = (textureLeftTop_.y + textureSize_.y) / static_cast<float>(metadata.height);

	// 左右反転
	if (isFlipX_) {
		left = -left;
		right = -right;
	}
	
	// 上下反転
	if (isFlipY_) {
		top = -top;
		bottom = -bottom;
	}

	// 頂点データの更新（左下、左上、右下、右上の順）
	vertexData_[0].position = { left, bottom, kSpriteDepth, kHomogeneousW };
	vertexData_[0].texcoord = { tex_left, tex_bottom };
	vertexData_[0].normal = kSpriteNormal;

	vertexData_[1].position = { left, top, kSpriteDepth, kHomogeneousW };
	vertexData_[1].texcoord = { tex_left, tex_top };
	vertexData_[1].normal = kSpriteNormal;

	vertexData_[2].position = { right, bottom, kSpriteDepth, kHomogeneousW };
	vertexData_[2].texcoord = { tex_right, tex_bottom };
	vertexData_[2].normal = kSpriteNormal;

	vertexData_[3].position = { right, top, kSpriteDepth, kHomogeneousW };
	vertexData_[3].texcoord = { tex_right, tex_top };
	vertexData_[3].normal = kSpriteNormal;

	// インデックスデータの更新（三角形1、三角形2）
	indexData_[0] = kTriangle1Index0;
	indexData_[1] = kTriangle1Index1;
	indexData_[2] = kTriangle1Index2;
	indexData_[3] = kTriangle2Index0;
	indexData_[4] = kTriangle2Index1;
	indexData_[5] = kTriangle2Index2;

	// Transform変数を作る
	Transform transform{
		{size_.x, size_.y, kSpriteScaleZ},
		{0.0f, 0.0f, rotation_},
		{position_.x, position_.y, kSpriteDepth}
	};

	// ワールド・ビュー・プロジェクション行列の計算
	Matrix4x4 worldMatrix = MakeAffineMatrix::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 viewMatrix = MakeIdentity4x4::MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix::MakeOrthographicMatrix(
		0.0f, 0.0f,
		static_cast<float>(WinApp::kClientWidth),
		static_cast<float>(WinApp::kClientHeight),
		SpriteDefaults::kOrthoNear,
		SpriteDefaults::kOrthoFar);

	transformationMatrixData_->wvp = Multiply::Multiply(worldMatrix, Multiply::Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData_->world = worldMatrix;
}

void Sprite::Draw()
{
	// 頂点バッファビューを設定
	dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
	
	// インデックスバッファビューを設定
	dxCommon_->GetCommandList()->IASetIndexBuffer(&indexBufferView_);

	// マテリアルCBufferの場所を設定（RootParameter配列の0番目）
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

	// WVP用CBufferの場所を設定（RootParameter配列の1番目）
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());

	// テクスチャDescriptorTableを設定（RootParameter配列の2番目）
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(filePath_));
	
	// 描画コマンド（インデックス付き描画）
	dxCommon_->GetCommandList()->DrawIndexedInstanced(SpriteDefaults::kIndexCount, 1, 0, 0, 0);
}

Microsoft::WRL::ComPtr<ID3D12Resource> Sprite::CreateBufferResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, size_t sizeInBytes)
{
	// DXGIファクトリーの生成
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(hr));

	// ヒーププロパティの設定（UploadHeap）
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	// リソースデスクの設定（バッファリソース）
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// リソースの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	hr = device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;
}

void Sprite::CreateVertexData()
{
	// 頂点リソースの作成
	vertexResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * SpriteDefaults::kVertexCount);
	
	// インデックスリソースの作成
	indexResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(uint32_t) * SpriteDefaults::kIndexCount);

	// 頂点バッファビューの設定
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * SpriteDefaults::kVertexCount;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// インデックスバッファビューの設定
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * SpriteDefaults::kIndexCount;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	// 頂点データのマッピング
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	// インデックスデータのマッピング
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
}

void Sprite::CreateMaterialData()
{
	// マテリアルリソースの作成
	materialResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(Material));

	// マテリアルデータのマッピング
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	// 初期カラーの設定（白）
	materialData_->color = SpriteDefaults::kInitColor;

	// スプライトはライティングしない
	materialData_->enableLighting = false;

	// UV変換行列を単位行列で初期化
	materialData_->uvTransform = MakeIdentity4x4::MakeIdentity4x4();
}

void Sprite::CreateWVPData()
{
	// WVPリソースの作成
	wvpResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformationMatrix));

	// WVPデータのマッピング
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));

	// 単位行列で初期化
	transformationMatrixData_->wvp = MakeIdentity4x4::MakeIdentity4x4();
	transformationMatrixData_->world = MakeIdentity4x4::MakeIdentity4x4();
}

void Sprite::AdjustTextureSize()
{
	// テクスチャメタデータを取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(filePath_);

	// テクスチャサイズを設定
	textureSize_.x = static_cast<float>(metadata.width);
	textureSize_.y = static_cast<float>(metadata.height);

	// スプライトサイズをテクスチャサイズに合わせる
	size_ = textureSize_;
}

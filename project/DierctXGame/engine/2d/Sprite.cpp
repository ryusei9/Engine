#include "Sprite.h"
#include "SpriteCommon.h"
#include "Transform.h"
#include "MakeAffineMatrix.h"
#include "MakePerspectiveFovMatrix.h"
#include "Multiply.h"
#include "MakeOrthographicMatrix.h"

/// 初期化
/// - dxCommon を保存し、テクスチャパスをメンバに保持する
/// - 頂点 / マテリアル / WVP のバッファを作成し、テクスチャサイズに合わせてスプライトサイズを設定する
/// - 副作用: GPU 用バッファを確保して Map する
void Sprite::Initialize(DirectXCommon* dxCommon, std::string textureFilePath)
{
	dxCommon_ = dxCommon;
	this->filePath = textureFilePath; // メンバー変数に値を設定
	
	// 頂点データの作成
	CreateVertexData();

	// Materialデータの作成
	CreateMaterialData();

	// WVPデータの作成
	CreateWVPData();

	// テクスチャサイズをイメージに合わせる
	AdjustTextureSize();
}

/// 毎フレーム更新
/// - アンカーポイント・テクスチャ領域に基づいて頂点／インデックスデータを更新する
/// - ワールド行列（スプライトの位置・回転・スケール）を計算して WVP を更新する
/// - 入力: メンバ変数 `position`/`size`/`rotation`/`anchorPoint`/`textureLeftTop`/`textureSize`
/// - 出力: `vertexData`（GPU マップ済み領域）、`transformationMatrixData`（WVP）
void Sprite::Update()
{
	float left = 0.0f - anchorPoint.x;
	float right = 1.0f - anchorPoint.x;
	float top = 0.0f - anchorPoint.y;
	float bottom = 1.0f - anchorPoint.y;

	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(filePath);
	float tex_left = textureLeftTop.x / metadata.width;
	float tex_right = (textureLeftTop.x + textureSize.x) / metadata.width;
	float tex_top = textureLeftTop.y / metadata.height;
	float tex_bottom = (textureLeftTop.y + textureSize.y) / metadata.height;

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

	// 頂点リソースにデータを書き込む
	vertexData[0].position = { left,bottom,0.0f,1.0f };
	vertexData[0].texcoord = { tex_left,tex_bottom };
	vertexData[0].normal = { 0.0f,0.0f,-1.0f };
	vertexData[1].position = { left,top,0.0f,1.0f };
	vertexData[1].texcoord = { tex_left,tex_top };
	vertexData[1].normal = { 0.0f,0.0f,-1.0f };
	vertexData[2].position = { right,bottom,0.0f,1.0f };
	vertexData[2].texcoord = { tex_right,tex_bottom };
	vertexData[2].normal = { 0.0f,0.0f,-1.0f };
	vertexData[3].position = { right,top,0.0f,1.0f };
	vertexData[3].texcoord = { tex_right,tex_top };
	vertexData[3].normal = { 0.0f,0.0f,-1.0f };

	// インデックスリソースにデータを書き込む
	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;

	// Transform変数を作る
	Transform transform{
		{size.x,size.y,1.0f},
		{0.0f,0.0f,rotation},
		{position.x,position.y,0.0f}
	};

	Matrix4x4 worldMatrix = MakeAffineMatrix::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 viewMatrix = MakeIdentity4x4::MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix::MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);


	transformationMatrixData->WVP = Multiply::Multiply(worldMatrix, Multiply::Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData->World = worldMatrix;
}

/// 描画
/// - 頂点／インデックスビューをコマンドリストにバインドし、マテリアル・WVP・テクスチャをルートに設定して描画コマンドを発行する
/// - 前提: `vertexResource`／`indexResource`／`materialResource`／`wvpResource` が有効であること
void Sprite::Draw()
{
	// VBVを設定
	dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	dxCommon_->GetCommandList()->IASetIndexBuffer(&indexBufferView);

	// 第一引数の0はRootParameter配列の0番目
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	// wvp用のCBufferの場所を設定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());

	// Spriteを常にuvCheckerにする
	//auto handle = TextureManager::GetInstance()->GetSrvHandleGPU(filePath);
	//printf("SRV Handle: %llu\n", handle.ptr);
	
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(filePath));
	// 描画
	dxCommon_->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

/// GPU 用バッファ作成ユーティリティ
/// - device に対して Upload ヒープでコミット済みバッファを作成し返す
/// - 成功を assert しているため、呼び出し元は失敗しない前提で使用する
Microsoft::WRL::ComPtr<ID3D12Resource> Sprite::CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes)
{
	// DXGIファクトリーの生成
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	// 関数が成功したかどうか
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};

	// UploadHeapを使う
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	// 頂点リソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};

	// バッファリソース。テクスチャの場合はまた別の設定をする
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	// リソースのサイズ
	resourceDesc.Width = sizeInBytes;

	// バッファの場合はこれらは1にする決まり
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;

	// バッファの場合はこれにする決まり
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;



	// 実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

/// 頂点 / インデックス用バッファの作成と Map
/// - vertexResource/indexResource を確保し、vertexData/indexData へマップする
/// - VertexData 構造のレイアウトに合わせて VertexBufferView / IndexBufferView を設定する
void Sprite::CreateVertexData()
{
	vertexResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * 6);
	indexResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(uint32_t) * 6);

	

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6;
	// 1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// リソースの先頭のアドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();

	// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;

	// インデックスはuint_32とする
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// 頂点データをリソースにコピー
	//std::memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());

	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	
}

/// マテリアル用定数バッファの作成と初期化
/// - materialResource を確保し materialData にマップしてデフォルト値を書き込む
void Sprite::CreateMaterialData()
{
	materialResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(Material));

	// ...Mapしてデータを書き込む。色は白を設定しておくといい
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	// SpriteはLightingしないのでfalseを設定する
	materialData->enableLighting = false;

	// 単位行列で初期化
	materialData->uvTransform = MakeIdentity4x4::MakeIdentity4x4();
}

/// WVP 用定数バッファの作成
/// - wvpResource を確保し transformationMatrixData をマップして単位行列で初期化する
void Sprite::CreateWVPData()
{
	wvpResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformationMatrix));

	// 書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));

	// 単位行列を書き込んでおく
	transformationMatrixData->WVP = MakeIdentity4x4::MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4::MakeIdentity4x4();
}

/// テクスチャ情報からスプライトの `size` を自動設定する
/// - TextureManager からテクスチャメタデータを取得して `textureSize` に反映
/// - 初期はスプライトの `size` をテクスチャサイズに合わせる
void Sprite::AdjustTextureSize()
{
	// テクスチャメタデータを取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(filePath);

	textureSize.x = static_cast<float>(metadata.width);
	textureSize.y = static_cast<float>(metadata.height);
	// 画像サイズをテクスチャサイズに合わせる
	size = textureSize;
}

#include "TextureManager.h"
#include "StringUtility.h"
#include <cassert>
#include "DirectXCommon.h"
std::shared_ptr<TextureManager> TextureManager::instance = nullptr;

// ImGuiで0番を使用するため、1番から使用
uint32_t TextureManager::kSRVIndexTop = 1;

void TextureManager::LoadTexture(const std::string& filePath)
{
	// 読み込み済みテクスチャを検索
	if (textureDatas.contains(filePath)) {
		// テクスチャ枚数上限チェック
		assert(textureDatas.size() + kSRVIndexTop < DirectXCommon::kMaxSRVCount);
		if (textureDatas.find(filePath) != textureDatas.end()) {
			// 読み込み済みなら早期return
			return;
		}
	}
	// テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr;
	if (filePathW.ends_with(L".dds")) {
		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	} else {
		hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}
	assert(SUCCEEDED(hr));

	// ミップマップの作成
	DirectX::ScratchImage mipImages{};
	if(DirectX::IsCompressed(image.GetMetadata().format)) {
		// 圧縮テクスチャの場合はミップマップを生成しない
		mipImages = std::move(image);
	} else {
		// 非圧縮テクスチャの場合はミップマップを生成する
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipImages);
		assert(SUCCEEDED(hr));
	}
	

	// ミップマップの結果を使用する
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	const DirectX::Image* mipImage = mipImages.GetImages();

	// テクスチャデータを追加
	//textureDatas.resize(textureDatas.size() + 1);
	TextureData& textureData = textureDatas[filePath];

	//textureData.filePath = filePath;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = dxCommon_->CreateTextureResource(textureData.metadata);

	// ミップマップデータをリソースにアップロード
	//dxCommon_->UploadTextureData(textureData.resource.Get(),mipImages);



	// テクスチャデータの要素数番号をSRVのインデックスにする
	//uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1) + kSRVIndexTop;
	textureData.srvIndex = srvManager_->Allocate();
	textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	if (metadata.IsCubemap()) {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.MipLevels = UINT_MAX;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	} else {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	}
	// SRVを作成
	dxCommon_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

	Microsoft::WRL::ComPtr< ID3D12Resource> intermediateResource = dxCommon_->UploadTextureData(textureData.resource.Get(), mipImages);
	// GPUとCPUの同期を待つ(これだと高速で処理ができないが高速にしようとすると手間なので応急処置)
	dxCommon_->SyncCPUWithGPU();
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath)
{
	// 範囲外指定違反チェック
	assert(textureDatas.contains(filePath));

	TextureData& textureData = textureDatas[filePath];
	return textureData.metadata;
}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filePath)
{
	// 読み込み済みテクスチャを検索
	if (textureDatas.contains(filePath)) {
		// `find` を使用してイテレータを取得
		auto it = textureDatas.find(filePath);

		// イテレータの位置を基にインデックスを計算
		uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
		return textureIndex;
	}

	assert(0);
	return 0;
}

void TextureManager::Initialize(SrvManager* srvManager)
{
	dxCommon_ = DirectXCommon::GetInstance();
	srvManager_ = srvManager;
	// SRVの数と同数
	textureDatas.reserve(DirectXCommon::kMaxSRVCount);
}

std::shared_ptr<TextureManager> TextureManager::GetInstance()
{
	if (instance == nullptr) {
		instance = std::make_shared<TextureManager>();
	}
	return instance;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath)
{
	// 範囲外指定違反チェック
	assert(textureDatas.contains(filePath));

	// テクスチャデータの参照を取得
	TextureData& textureData = textureDatas[filePath];

	// GPU ハンドルを返す
	return textureData.srvHandleGPU;
}

void TextureManager::Finalize()
{
	
}

#include "TextureManager.h"
#include "StringUtility.h"
#include <cassert>
#include "DirectXCommon.h"

//
// TextureManager
// - テクスチャの読み込み・管理を行うシングルトン。
// - 機能概要：ファイルからテクスチャを読み込み（WIC / DDS）、必要に応じてミップマップを生成し、
//   Direct3D のリソースへアップロードして SRV（Shader Resource View）を作成・管理する。
// - 設計メモ：
//   * textureDatas_ はファイルパスをキーにした連想配列（unordered_map）で、読み込み済みテクスチャを保持する。
//   * SRV のヒープインデックスは内部で srvManager_ に Allocate させ、そのハンドルを保存する。
//   * ImGui 利用の都合で 0 番を予約しており、実際のテクスチャは kSRVIndexTop (=1) から割り当てる。
//   * 圧縮テクスチャ（BCn 等）はミップ生成を行わずそのまま利用する。非圧縮テクスチャは GenerateMipMaps を使う。
//   * 実装は簡易化のためアップロード同期（SyncCPUWithGPU）を呼び出している。大規模ロードやストリーミング化する場合は
//     非同期アップロード＋複数バッファ戦略に変更することを推奨する。
//   * 呼び出し側は Initialize(SrvManager*) を呼んでから使用すること。
//   * Finalize() は将来的なリソース解放処理を入れるフック（現状内容なし）。
//

using namespace TextureManagerConstants;

std::shared_ptr<TextureManager> TextureManager::instance_ = nullptr;
uint32_t TextureManager::kSRVIndexTop_ = kSRVIndexTop;

std::shared_ptr<TextureManager> TextureManager::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = std::make_shared<TextureManager>();
	}
	return instance_;
}

void TextureManager::Initialize(SrvManager* srvManager)
{
	assert(srvManager != nullptr && "SrvManager pointer is null!");
	
	// DirectXCommon / SrvManager の参照を保持
	dxCommon_ = DirectXCommon::GetInstance();
	srvManager_ = srvManager;
	
	// 内部コンテナの予約
	textureDatas_.reserve(srvManager_->GetMaxSRVCount());
}

void TextureManager::Finalize()
{
	// リソースの解放
	textureDatas_.clear();
}

void TextureManager::LoadTexture(const std::string& filePath)
{
	// 既に読み込まれている場合は早期リターン
	if (IsTextureExists(filePath)) {
		return;
	}

	// テクスチャ枚数上限チェック
	assert(CanLoadMoreTextures() && "Cannot load more textures, SRV heap is full!");

	// ファイル読み込み
	DirectX::ScratchImage image = LoadTextureFile(filePath);

	// ミップマップ生成
	DirectX::ScratchImage mipImages = GenerateMipMapsIfNeeded(image);

	// TextureData を作成
	TextureData& textureData = textureDatas_[filePath];
	
	// リソース作成
	CreateTextureResource(textureData, mipImages);
	
	// SRV作成
	CreateShaderResourceView(textureData, mipImages.GetMetadata());
	
	// GPU へアップロード
	UploadTextureToGPU(textureData, mipImages);
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath)
{
	assert(IsTextureExists(filePath) && "Texture not found!");
	return textureDatas_[filePath].metadata;
}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filePath)
{
	assert(IsTextureExists(filePath) && "Texture not found!");
	
	auto it = textureDatas_.find(filePath);
	uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas_.begin(), it));
	return textureIndex;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath)
{
	assert(IsTextureExists(filePath) && "Texture not found!");
	
	TextureData& textureData = textureDatas_[filePath];
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);
	return textureData.srvHandleGPU;
}

bool TextureManager::IsTextureLoaded(const std::string& filePath) const
{
	return IsTextureExists(filePath);
}

// ===== ヘルパー関数 =====

DirectX::ScratchImage TextureManager::LoadTextureFile(const std::string& filePath)
{
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr;

	if (IsDDSFile(filePathW)) {
		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	} else {
		hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}

	if (!SUCCEEDED(hr)) {
		OutputDebugStringA(("Texture load failed: " + filePath + "\n").c_str());
		OutputDebugStringA(("HRESULT: " + std::to_string(hr) + "\n").c_str());
		assert(false && "Failed to load texture file!");
	}

	return image;
}

DirectX::ScratchImage TextureManager::GenerateMipMapsIfNeeded(DirectX::ScratchImage& image)
{
	DirectX::ScratchImage mipImages{};

	if (DirectX::IsCompressed(image.GetMetadata().format)) {
		// 圧縮テクスチャはそのまま使用
		mipImages = std::move(image);
	} else {
		// 非圧縮テクスチャは自動でミップマップを生成
		HRESULT hr = DirectX::GenerateMipMaps(
			image.GetImages(), 
			image.GetImageCount(), 
			image.GetMetadata(), 
			DirectX::TEX_FILTER_DEFAULT, 
			kDefaultGenerateMipLevel, 
			mipImages);
		assert(SUCCEEDED(hr) && "Failed to generate mipmaps!");
	}

	return mipImages;
}

void TextureManager::CreateTextureResource(TextureData& textureData, const DirectX::ScratchImage& mipImages)
{
	// メタデータを保存
	textureData.metadata = mipImages.GetMetadata();
	
	// リソースを作成
	textureData.resource = dxCommon_->CreateTextureResource(textureData.metadata);
	
	// SRV インデックスを取得
	textureData.srvIndex = srvManager_->Allocate();
	textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);
}

void TextureManager::CreateShaderResourceView(TextureData& textureData, const DirectX::TexMetadata& metadata)
{
	// SRV 設定を作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = CreateSRVDesc(metadata);

	// SRV をデスクリプタヒープに作成
	dxCommon_->GetDevice()->CreateShaderResourceView(
		textureData.resource.Get(), 
		&srvDesc, 
		textureData.srvHandleCPU);
}

void TextureManager::UploadTextureToGPU(const TextureData& textureData, const DirectX::ScratchImage& mipImages)
{
	// テクスチャデータを GPU にアップロード
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = 
		dxCommon_->UploadTextureData(textureData.resource.Get(), mipImages);

	// CPU と GPU の同期を待つ
	dxCommon_->SyncCPUWithGPU();
}

D3D12_SHADER_RESOURCE_VIEW_DESC TextureManager::CreateSRVDesc(const DirectX::TexMetadata& metadata) const
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	if (metadata.IsCubemap()) {
		// キューブマップの設定
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip = kDefaultMostDetailedMip;
		srvDesc.TextureCube.MipLevels = kDefaultMipLevels;
		srvDesc.TextureCube.ResourceMinLODClamp = kDefaultResourceMinLODClamp;
	} else {
		// 2Dテクスチャの設定
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);
	}

	return srvDesc;
}

bool TextureManager::IsDDSFile(const std::wstring& filePath) const
{
	return filePath.ends_with(kDDSExtension);
}

bool TextureManager::IsTextureExists(const std::string& filePath) const
{
	return textureDatas_.contains(filePath);
}

bool TextureManager::CanLoadMoreTextures() const
{
	return textureDatas_.size() + kSRVIndexTop < srvManager_->GetMaxSRVCount();
}

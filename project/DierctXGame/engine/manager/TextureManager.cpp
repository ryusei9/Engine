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
//   * textureDatas はファイルパスをキーにした連想配列（unordered_map）で、読み込み済みテクスチャを保持する。
//   * SRV のヒープインデックスは内部で srvManager_ に Allocate させ、そのハンドルを保存する。
//   * ImGui 利用の都合で 0 番を予約しており、実際のテクスチャは kSRVIndexTop (=1) から割り当てる。
//   * 圧縮テクスチャ（BCn 等）はミップ生成を行わずそのまま利用する。非圧縮テクスチャは GenerateMipMaps を使う。
//   * 実装は簡易化のためアップロード同期（SyncCPUWithGPU）を呼び出している。大規模ロードやストリーミング化する場合は
//     非同期アップロード＋複数バッファ戦略に変更することを推奨する。
//   * 呼び出し側は Initialize(SrvManager*) を呼んでから使用すること。
//   * Finalize() は将来的なリソース解放処理を入れるフック（現状内容なし）。
//

std::shared_ptr<TextureManager> TextureManager::instance = nullptr;

// ImGui で 0 番を予約して使うため、実際のテクスチャは 1 番から使用する
uint32_t TextureManager::kSRVIndexTop = 1;

void TextureManager::LoadTexture(const std::string& filePath)
{
	// 既に読み込まれている場合は早期リターン
	if (textureDatas.contains(filePath)) {
		// テクスチャ枚数上限チェック（SRV ヒープの上限を超えないようにする）
		assert(textureDatas.size() + kSRVIndexTop < DirectXCommon::kMaxSRVCount);
		if (textureDatas.find(filePath) != textureDatas.end()) {
			// 既にロード済み
			return;
		}
	}

	// ファイル読み込み（WIC / DDS を自動選択）
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr;
	if (filePathW.ends_with(L".dds")) {
		// **デバッグ: メタデータを事前に確認**
		DirectX::TexMetadata metadata;
		hr = DirectX::GetMetadataFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, metadata);
		if (SUCCEEDED(hr)) {
			char debugInfo[512];
			sprintf_s(debugInfo, "[DDS Load] Path:%s, IsCubemap:%d, ArraySize:%zu, Format:%d\n",
				filePath.c_str(), metadata.IsCubemap(), metadata.arraySize, metadata.format);
			OutputDebugStringA(debugInfo);
		}
		
		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	} else {
		hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}
	if (!SUCCEEDED(hr)) {
		OutputDebugStringA(("Texture load failed: " + filePath + "\n").c_str());
		char hrHex[64];
		sprintf_s(hrHex, "HRESULT: 0x%08X\n", static_cast<unsigned int>(hr));
		OutputDebugStringA(hrHex);
	}
	assert(SUCCEEDED(hr));

	// ミップマップ生成
	DirectX::ScratchImage mipImages{};
	if (DirectX::IsCompressed(image.GetMetadata().format)) {
		// 圧縮テクスチャはそのまま使用（GenerateMipMaps は圧縮フォーマット非対応のことが多い）
		mipImages = std::move(image);
	} else {
		// 非圧縮テクスチャは自動でミップマップを生成
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipImages);
		assert(SUCCEEDED(hr));
	}

	// 使用するメタデータ／イメージ配列を参照
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	const DirectX::Image* mipImage = mipImages.GetImages();

	// **デバッグ: キューブマップかどうか確認**
	if (metadata.IsCubemap()) {
		OutputDebugStringA(("Creating CUBEMAP SRV for: " + filePath + "\n").c_str());
	} else {
		OutputDebugStringA(("Creating TEXTURE2D SRV for: " + filePath + "\n").c_str());
	}

	// textureDatas に要素を追加して参照を取得（デフォルト構築を利用）
	TextureData& textureData = textureDatas[filePath];

	// メタデータを保存し、リソースを作成
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = dxCommon_->CreateTextureResource(textureData.metadata);

	// SRV インデックスを取得してハンドルを計算・保存
	textureData.srvIndex = srvManager_->Allocate();
	textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);

	// SRV 設定を作る（2D or Cube に応じて設定）
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

	// SRV をデスクリプタヒープに作成
	dxCommon_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

	// テクスチャデータを GPU にアップロード（中間リソースを返す）
	Microsoft::WRL::ComPtr< ID3D12Resource> intermediateResource = dxCommon_->UploadTextureData(textureData.resource.Get(), mipImages);

	// 簡易実装: CPU と GPU の同期を待つ（同期してしまうため高速化は未対応）
	dxCommon_->SyncCPUWithGPU();
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath)
{
	// ファイルが存在することのチェック
	assert(textureDatas.contains(filePath));

	TextureData& textureData = textureDatas[filePath];
	return textureData.metadata;
}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filePath)
{
	// 読み込み済みテクスチャのインデックス（内部 map の順序に基づく）を返す
	if (textureDatas.contains(filePath)) {
		auto it = textureDatas.find(filePath);
		uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
		return textureIndex;
	}

	// 存在しない場合はアサート（呼び出し側で存在確認すること）
	assert(0);
	return 0;
}

void TextureManager::Initialize(SrvManager* srvManager)
{
	// DirectXCommon / SrvManager の参照を保持
	dxCommon_ = DirectXCommon::GetInstance();
	srvManager_ = srvManager;
	// 内部コンテナの予約（SRV の最大数分）
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
	// ファイルが存在することのチェック
	assert(textureDatas.contains(filePath));

	// 対応する TextureData を取得して GPU ハンドルを返す
	TextureData& textureData = textureDatas[filePath];

	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);
	return textureData.srvHandleGPU;
}

void TextureManager::Finalize()
{
	// 将来的なリソース解放処理のフック（現状は何もしない）
}

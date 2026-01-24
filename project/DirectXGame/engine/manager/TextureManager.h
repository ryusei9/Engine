#pragma once
#include <string>
#include <DirectXTex.h>
#include <wrl.h>
#include <d3d12.h>
#include <SrvManager.h>
#include <unordered_map>
#include <memory>

class DirectXCommon;

// TextureManager用の定数
namespace TextureManagerConstants {
	// SRVインデックスの開始番号（ImGuiで0番を予約）
	constexpr uint32_t kSRVIndexTop = 1;
	
	// デフォルトのMipLevels設定
	constexpr UINT kDefaultMipLevels = UINT_MAX;
	
	// デフォルトのMostDetailedMip
	constexpr UINT kDefaultMostDetailedMip = 0;
	
	// デフォルトのResourceMinLODClamp
	constexpr float kDefaultResourceMinLODClamp = 0.0f;
	
	// デフォルトのGenerateMipMapsレベル
	constexpr size_t kDefaultGenerateMipLevel = 0;
	
	// DDSファイル拡張子
	constexpr const wchar_t* kDDSExtension = L".dds";
}

/// <summary>
/// テクスチャマネージャー
/// </summary>
class TextureManager
{
public:
	// コンストラクタ・デストラクタ
	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;

	// シングルトンインスタンスの取得
	static std::shared_ptr<TextureManager> GetInstance();

	// 初期化
	void Initialize(SrvManager* srvManager);

	// 終了
	void Finalize();

	// テクスチャファイルの読み込み
	void LoadTexture(const std::string& filePath);

	// テクスチャインデックスの取得
	uint32_t GetTextureIndexByFilePath(const std::string& filePath);

	// テクスチャ番号からGPUハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string& filePath);

	// メタデータを取得
	const DirectX::TexMetadata& GetMetaData(const std::string& filePath);

	// ゲッター
	bool IsTextureLoaded(const std::string& filePath) const;
	size_t GetLoadedTextureCount() const { return textureDatas_.size(); }
	uint32_t GetSRVIndexTop() const { return TextureManagerConstants::kSRVIndexTop; }

private:
	// シングルトンインスタンス
	static std::shared_ptr<TextureManager> instance_;

	// テクスチャ1枚分のデータ
	struct TextureData {
		DirectX::TexMetadata metadata;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		uint32_t srvIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
	};

	// テクスチャ読み込みヘルパー
	DirectX::ScratchImage LoadTextureFile(const std::string& filePath);
	DirectX::ScratchImage GenerateMipMapsIfNeeded(DirectX::ScratchImage& image);
	void CreateTextureResource(TextureData& textureData, const DirectX::ScratchImage& mipImages);
	void CreateShaderResourceView(TextureData& textureData, const DirectX::TexMetadata& metadata);
	void UploadTextureToGPU(const TextureData& textureData, const DirectX::ScratchImage& mipImages);
	
	// SRV記述子の設定
	D3D12_SHADER_RESOURCE_VIEW_DESC CreateSRVDesc(const DirectX::TexMetadata& metadata) const;
	
	// ファイルタイプの判定
	bool IsDDSFile(const std::wstring& filePath) const;
	
	// テクスチャ存在チェック
	bool IsTextureExists(const std::string& filePath) const;
	
	// テクスチャ枚数上限チェック
	bool CanLoadMoreTextures() const;

	// テクスチャデータ
	std::unordered_map<std::string, TextureData> textureDatas_;

	// DirectXCommon
	DirectXCommon* dxCommon_ = nullptr;

	// SRVManager
	SrvManager* srvManager_ = nullptr;

	// 後方互換性のための静的メンバ
	static uint32_t kSRVIndexTop_;
};


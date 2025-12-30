#pragma once
#include <string>
#include <DirectXTex.h>
#include <wrl.h>
#include <d3d12.h>
#include <SrvManager.h>
#include <unordered_map>
#include <memory>

class DirectXCommon;// 前方宣言
/// <summary>
/// テクスチャマネージャー
/// </summary>
class TextureManager
{
public:
	// 初期化
	void Initialize(SrvManager* srvManager);

	// シングルトンインスタンスの取得
	static std::shared_ptr<TextureManager> GetInstance();

	// コンストラクタ・デストラクタ
	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;

	// 終了
	void Finalize();

	/// <summary>
	/// テクスチャファイルの読み込み
	/// </summary>
	void LoadTexture(const std::string& filePath);

	// SRVインデックスの開始番号
	uint32_t GetTextureIndexByFilePath(const std::string& filePath);

	// テクスチャ番号からGPUハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string& filePath);

	// メタデータを取得
	const DirectX::TexMetadata& GetMetaData(const std::string& filePath);

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

	// テクスチャデータ
	std::unordered_map<std::string, TextureData> textureDatas_;

	DirectXCommon* dxCommon_;

	// SRVインデックスの開始番号
	static uint32_t kSRVIndexTop_;

	SrvManager* srvManager_;
};


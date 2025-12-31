#pragma once
#include <map>
#include <string>
#include <memory>
#include "Model.h"
#include <unordered_map>
#include "ModelCommon.h"
#include "DirectXCommon.h"

// ModelManager用の定数
namespace ModelManagerConstants {
	// デフォルトのリソースディレクトリ
	constexpr const char* kDefaultResourceDirectory = "resources";
}

/// <summary>
/// モデルマネージャー
/// </summary>
class ModelManager
{
public:
	// シングルトンインスタンスの取得
	static std::shared_ptr<ModelManager> GetInstance();
	
	// コンストラクタ・デストラクタ
	ModelManager() = default;
	~ModelManager() = default;
	
	// コピー禁止
	ModelManager(const ModelManager&) = delete;
	ModelManager& operator=(const ModelManager&) = delete;

	// 初期化
	void Initialize();

	// モデルの読み込み
	void LoadModel(const std::string& filePath);

	// 終了
	void Finalize();

	// モデルの取得
	Model* GetModel(const std::string& fileName);

	// モデルの検索
	Model* FindModel(const std::string& filePath);

private:
	// ヘルパー関数
	bool IsModelLoaded(const std::string& filePath) const;
	std::unique_ptr<Model> CreateAndInitializeModel(const std::string& filePath);
	void RegisterModel(const std::string& filePath, std::unique_ptr<Model> model);

	// シングルトンインスタンス
	static std::shared_ptr<ModelManager> sInstance_;
	
	// モデル共通部
	std::unique_ptr<ModelCommon> modelCommon_ = nullptr;
	
	// モデルデータ
	std::unordered_map<std::string, std::unique_ptr<Model>> models_;
};






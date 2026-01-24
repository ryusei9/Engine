#pragma once
#include <map>
#include <string>
#include <memory>
#include <unordered_map>
#include "Model.h"
#include "ModelCommon.h"
#include "DirectXCommon.h"

/// <summary>
/// ModelManager用の定数
/// </summary>
namespace ModelManagerConstants {
	// デフォルトのリソースディレクトリ
	constexpr const char* kDefaultResourceDirectory = "resources";
}

/// <summary>
/// モデルマネージャー（シングルトン）
/// 3Dモデルの読み込みと管理を行う
/// </summary>
class ModelManager
{
public:
	/*------メンバ関数------*/

	// シングルトンインスタンスの取得
	static std::shared_ptr<ModelManager> GetInstance();
	
	// コンストラクタ・デストラクタ
	ModelManager() = default;
	~ModelManager() = default;
	
	// コピー・ムーブ禁止
	ModelManager(const ModelManager&) = delete;
	ModelManager& operator=(const ModelManager&) = delete;
	ModelManager(ModelManager&&) = delete;
	ModelManager& operator=(ModelManager&&) = delete;

	// 初期化
	void Initialize();

	// モデルの読み込み
	void LoadModel(const std::string& filePath);

	// 終了処理
	void Finalize();

	// モデルの取得
	Model* GetModel(const std::string& fileName);

	// モデルの検索
	Model* FindModel(const std::string& filePath);

private:
	/*------ヘルパー関数------*/

	// モデルが読み込み済みかチェック
	bool IsModelLoaded(const std::string& filePath) const;

	// モデルを作成して初期化
	std::unique_ptr<Model> CreateAndInitializeModel(const std::string& filePath);

	// モデルをマップに登録
	void RegisterModel(const std::string& filePath, std::unique_ptr<Model> model);

	/*------メンバ変数------*/

	// シングルトンインスタンス
	static std::shared_ptr<ModelManager> sInstance_;
	
	// モデル共通部
	std::unique_ptr<ModelCommon> modelCommon_;
	
	// モデルデータマップ（ファイルパスをキーとする）
	std::unordered_map<std::string, std::unique_ptr<Model>> models_;
};






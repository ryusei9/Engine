#pragma once
#include <map>
#include <string>
#include <memory>
#include "Model.h"
// textureマネージャー
class ModelManager
{
private:
	static ModelManager* instance;

	std::unique_ptr<ModelCommon> modelCommon = nullptr;

	ModelManager() = default;
	~ModelManager() = default;
	// コピーコンストラクタ
	ModelManager(ModelManager&) = default;

	// コピー代入演算子
	ModelManager& operator=(ModelManager&) = default;

	
public:
	// シングルトンインスタンスの取得
	static ModelManager* GetInstance();
	// 終了
	void Finalize();
	// 初期化
	void Initialize(DirectXCommon* dxCommon);

	/// モデルファイルの読み込み
	void LoadModel(const std::string& filePath);

	// モデルデータ
	std::map<std::string, std::unique_ptr<Model>> models;

	// モデルの検索
	Model* FindModel(const std::string& filePath);
};


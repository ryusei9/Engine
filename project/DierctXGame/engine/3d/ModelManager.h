#pragma once
#include <map>
#include <string>
#include <memory>
#include "Model.h"
#include <unordered_map>
#include "ModelCommon.h"
#include "DirectXCommon.h"

/// <summary>
/// モデルマネージャー
/// </summary>
class ModelManager
{
public:
    // シングルトンインスタンスの取得
    static std::shared_ptr<ModelManager> GetInstance();
    ModelManager() = default;
    ~ModelManager() = default;
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
   

    static std::shared_ptr<ModelManager> instance;
    std::unique_ptr<ModelCommon> modelCommon = nullptr;
    // モデルデータ
    std::unordered_map<std::string, std::unique_ptr<Model>> models;
};





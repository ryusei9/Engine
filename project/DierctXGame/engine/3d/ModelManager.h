#pragma once
#include <map>
#include <string>
#include <memory>
#include "Model.h"
#include <unordered_map>
#include "ModelCommon.h"
#include "DirectXCommon.h"
// textureマネージャー
class ModelManager
{
public:
    // シングルトンインスタンスの取得
    ModelManager() = default;
    ~ModelManager() = default;
    ModelManager(const ModelManager&) = delete;
    ModelManager& operator=(const ModelManager&) = delete;

    static std::shared_ptr<ModelManager> GetInstance();
    void Initialize();
    void LoadModel(const std::string& filePath);
   
    void Finalize();

	

	// モデルの検索
	Model* FindModel(const std::string& filePath);
private:
   

    static std::shared_ptr<ModelManager> instance;
    std::unique_ptr<ModelCommon> modelCommon = nullptr;
    // モデルデータ
    std::unordered_map<std::string, std::unique_ptr<Model>> models;
};





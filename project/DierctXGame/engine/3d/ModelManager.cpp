#include "ModelManager.h"


std::shared_ptr<ModelManager> ModelManager::instance = nullptr;

std::shared_ptr<ModelManager> ModelManager::GetInstance() {
	if (instance == nullptr) {
		instance = std::make_shared<ModelManager>();
	}
	return instance;
}

void ModelManager::Initialize()
{
	modelCommon = std::make_unique<ModelCommon>();
	modelCommon->Initialize(DirectXCommon::GetInstance());
}

void ModelManager::LoadModel(const std::string& filePath)
{
	// 読み込みモデルを検索
	if (models.contains(filePath)) {
		// 読み込み済なら早期return
		return;
	}
	// モデルの生成とファイル読み込み、初期化
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize(modelCommon.get(), "resources", filePath);

	// モデルをmapコンテナに格納する
	models.insert(std::make_pair(filePath, std::move(model)));
}

Model* ModelManager::FindModel(const std::string& filePath)
{
	// 読み込みモデルを検索
	if (models.contains(filePath)) {
		// 読み込みモデルを戻り値としてreturn
		return models.at(filePath).get();
	}
	// ファイル名一致無し
	return nullptr;
}

void ModelManager::Finalize()
{
	
}

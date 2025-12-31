#include "ModelManager.h"

namespace {
	using namespace ModelManagerConstants;
}

std::shared_ptr<ModelManager> ModelManager::sInstance_ = nullptr;

// シングルトン取得
// - 初回呼び出し時にインスタンスを生成して返す
std::shared_ptr<ModelManager> ModelManager::GetInstance()
{
	if (sInstance_ == nullptr) {
		sInstance_ = std::make_shared<ModelManager>();
	}
	return sInstance_;
}

// 初期化
// - ModelCommon を生成して初期化する（内部で DirectXCommon の参照を使う）
// - 呼び出しタイミング: アプリ開始時など 1 回
void ModelManager::Initialize()
{
	// ModelCommon を生成して初期化
	modelCommon_ = std::make_unique<ModelCommon>();
	modelCommon_->Initialize(DirectXCommon::GetInstance());
}

// モデル読み込み関数
// - filePath: "monster.obj" のようなファイル名（resources ディレクトリを想定）
// - 処理:
//   1) すでに map に読み込まれていれば何もしない（冪等）
//   2) 新規に Model を生成し Initialize で OBJ を読み込む
//   3) models_ マップに move して格納する（ModelManager が所有）
// - 副作用: TextureManager によるテクスチャ読み込み等は Model::Initialize 内で発生する
void ModelManager::LoadModel(const std::string& filePath)
{
	// 読み込み済みかチェック（冪等性の保証）
	if (IsModelLoaded(filePath)) {
		return;
	}

	// Model を生成して初期化
	std::unique_ptr<Model> model = CreateAndInitializeModel(filePath);

	// マップに登録（所有権を移動）
	RegisterModel(filePath, std::move(model));
}

// 読み込み済みモデルを取得
// - filePath に一致するモデルが登録されていればポインタを返す。
// - 見つからなければ nullptr を返す（呼び出し側で nullptr チェックが必要）。
Model* ModelManager::FindModel(const std::string& filePath)
{
	// マップから検索
	if (models_.contains(filePath)) {
		return models_.at(filePath).get();
	}
	return nullptr;
}

// 終了処理
// - 現状は明示的な処理なし。ただし将来的にリソース解放や map の clear を入れる場所
void ModelManager::Finalize()
{
	// 全モデルを解放
	models_.clear();
	modelCommon_.reset();
}

// 別名の取得関数（存在チェックを行う簡易ラッパ）
// - GetModel と FindModel の機能は重複するが、コードベースの呼び出し方に合わせて両方用意している
Model* ModelManager::GetModel(const std::string& fileName)
{
	// FindModel のエイリアス
	return FindModel(fileName);
}

// ===== ヘルパー関数 =====

bool ModelManager::IsModelLoaded(const std::string& filePath) const
{
	return models_.contains(filePath);
}

std::unique_ptr<Model> ModelManager::CreateAndInitializeModel(const std::string& filePath)
{
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize(modelCommon_.get(), kDefaultResourceDirectory, filePath);
	return model;
}

void ModelManager::RegisterModel(const std::string& filePath, std::unique_ptr<Model> model)
{
	models_.insert(std::make_pair(filePath, std::move(model)));
}

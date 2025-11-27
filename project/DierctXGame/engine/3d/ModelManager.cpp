		#include "ModelManager.h"

//
// ModelManager: 3Dモデルの読み込み・管理を行うシングルトン
// - 生のファイルパス（キー）でモデルを1つだけ読み込み、内部 map に保持する。
// - 他モジュールは GetInstance()->FindModel(filePath) でポインタを取得して描画等に利用する。
// - 実装の注意点:
//   - 現状はスレッドセーフではない（呼び出しはメインスレッド前提）
//   - モデルのライフタイムは ModelManager が所有する unique_ptr によって管理される
//

std::shared_ptr<ModelManager> ModelManager::instance = nullptr;

// シングルトン取得
// - 初回呼び出し時にインスタンスを生成して返す
std::shared_ptr<ModelManager> ModelManager::GetInstance() {
	if (instance == nullptr) {
		instance = std::make_shared<ModelManager>();
	}
	return instance;
}

// 初期化
// - ModelCommon を生成して初期化する（内部で DirectXCommon の参照を使う）
// - 呼び出しタイミング: アプリ開始時など 1 回
void ModelManager::Initialize()
{
	modelCommon = std::make_unique<ModelCommon>();
	modelCommon->Initialize(DirectXCommon::GetInstance());
}

// モデル読み込み関数
// - filePath: "monster.obj" のようなファイル名（resources ディレクトリを想定）
// - 処理:
//   1) すでに map に読み込まれていれば何もしない（冪等）
//   2) 新規に Model を生成し Initialize で OBJ を読み込む
//   3) models マップに move して格納する（ModelManager が所有）
// - 副作用: TextureManager によるテクスチャ読み込み等は Model::Initialize 内で発生する
void ModelManager::LoadModel(const std::string& filePath)
{
	// 読み込み済みかチェック（存在するなら再読み込みを行わない）
	if (models.contains(filePath)) {
		return; // 既にロード済み
	}

	// Model を生成して初期化（resources フォルダを基準にファイル読み込み）
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize(modelCommon.get(), "resources", filePath);

	// マップに格納（所有権を移動）
	models.insert(std::make_pair(filePath, std::move(model)));
}

// 読み込み済みモデルを取得
// - filePath に一致するモデルが登録されていればポインタを返す。
// - 見つからなければ nullptr を返す（呼び出し側で nullptr チェックが必要）。
Model* ModelManager::FindModel(const std::string& filePath)
{
	if (models.contains(filePath)) {
		return models.at(filePath).get();
	}
	return nullptr;
}

// 終了処理
// - 現状は明示的な処理なし。ただし将来的にリソース解放や map の clear を入れる場所
void ModelManager::Finalize()
{
	// ここで models を clear したり ModelCommon を解放したりできます。
	// 明示的な順序が必要ならここに実装してください。
	models.clear();
	modelCommon.reset();
}

// 別名の取得関数（存在チェックを行う簡易ラッパ）
// - GetModel と FindModel の機能は重複するが、コードベースの呼び出し方に合わせて両方用意している
Model* ModelManager::GetModel(const std::string& fileName)
{
	auto it = models.find(fileName);
	if (it != models.end()) {
		return it->second.get();
	}
	return nullptr;
}

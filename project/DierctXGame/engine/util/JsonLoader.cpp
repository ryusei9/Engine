#include "../util/JsonLoader.h"

LevelData* JsonLoader::Load(const std::string& fileName)
{
	// ここで初期化
	const std::string kDefaultBaseDirectory = "resources/"; // 必要に応じてパスを変更
	const std::string kExtension = ".json";
	// 連結してフルパスを得る
	const std::string fullpath = kDefaultBaseDirectory + fileName + kExtension;

	// ファイルストリーム
	std::ifstream file;

	// ファイルを開く
	file.open(fullpath);
	// ファイルオープン失敗チェック
	if (file.fail()) {
		assert(0);
	}
	// JSON文字列から回答したデータ
	nlohmann::json deserialized;

	// 解凍
	file >> deserialized;

	// 正しいレベルデータファイルかチェック
	assert(deserialized.is_object());
	assert(deserialized.contains("name"));
	assert(deserialized["name"].is_string());

	// "name"を文字列として取得
	std::string name = deserialized["name"].get<std::string>();
	// 正しいレベルデータファイルかチェック
	assert(name.compare("scene") == 0);

	// レベルデータ格納用インスタンスを生成
	LevelData* levelData = new LevelData();

	// "objects"の全オブジェクトを走査
	for (nlohmann::json& object : deserialized["objects"]) {
		assert(object.contains("type"));

		// 種類を取得
		std::string type = object["type"].get<std::string>();

		// MESH
		if(type.compare("MESH" ) == 0) {
			// 要素追加
			levelData->objects.emplace_back(LevelData::ObjectData{});
			// 今追加した要素の参照を得る
			LevelData::ObjectData& objectData = levelData->objects.back();

			if (object.contains("file_name")) {
				// ファイル名を取得
				objectData.fileName = object["file_name"];
			}
			// トランスフォームのパラメータ読み込み
			nlohmann::json& transform = object["transform"];
			// 平行移動
			objectData.translation.x = (float)transform["translation"][0];
			objectData.translation.y = (float)transform["translation"][2];
			objectData.translation.z = (float)transform["translation"][1];
			// 回転角
			objectData.rotation.x = -(float)transform["rotation"][0];
			objectData.rotation.y = -(float)transform["rotation"][2];
			objectData.rotation.z = -(float)transform["rotation"][1];
			// スケール
			objectData.scaling.x = (float)transform["scale"][0];
			objectData.scaling.y = (float)transform["scale"][2];
			objectData.scaling.z = (float)transform["scale"][1];
			// TODO: コライダーのパラメータ読み込み

			// TODO: オブジェクト走査を再帰関数にまとめ、再帰呼び出しで枝を走査する
			if (object.contains("children")) {

			}
		}
	}
	return levelData;
}

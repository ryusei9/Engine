#include "../util/JsonLoader.h"

LevelData* JsonLoader::Load(const std::string& fileName)
{
	const std::string kDefaultBaseDirectory = "resources/";
	const std::string kExtension = ".json";
	const std::string fullpath = kDefaultBaseDirectory + fileName + kExtension;

	std::ifstream file(fullpath);
	if (file.fail()) {
		assert(!"ファイルを開けませんでした");
	}

	nlohmann::json deserialized;
	file >> deserialized;

	// ベースチェック
	assert(deserialized.is_object());
	assert(deserialized.contains("name"));
	assert(deserialized["name"].is_string());

	std::string sceneName = deserialized["name"].get<std::string>();
	assert(sceneName == "scene");

	LevelData* levelData = new LevelData();

	if (deserialized.contains("objects") && deserialized["objects"].is_array()) {
		for (const auto& objectJson : deserialized["objects"]) {
			if (objectJson.contains("type") && objectJson["type"] == "MESH") {
                LevelData::ObjectData obj = ConvertJsonToObject(objectJson);
				levelData->objects.push_back(obj);
			} else if (objectJson.contains("type") && objectJson["type"] == "PlayerSpawn") {
				LevelData::PlayerData playerData;
				// プレイヤーの位置と回転を取得
				if (objectJson.contains("transform")) {
					const auto& transform = objectJson["transform"];
					playerData.translation.x = (float)transform["translation"][0];
					playerData.translation.y = (float)transform["translation"][2];
					playerData.translation.z = (float)transform["translation"][1];
					playerData.rotation.x = -(float)transform["rotation"][0];
					playerData.rotation.y = -(float)transform["rotation"][2];
					playerData.rotation.z = -(float)transform["rotation"][1];
				}
				// プレイヤーのデータを追加
				levelData->players.push_back(playerData);
			}
		}
	}

	return levelData;
}

LevelData::ObjectData JsonLoader::ConvertJsonToObject(const nlohmann::json& jsonNode)
{
	LevelData::ObjectData objectData;

	// disabled（無効フラグ）を読み込む
	if (jsonNode.contains("disabled") && jsonNode["disabled"].is_boolean()) {
		objectData.disabled = jsonNode["disabled"];
	} else {
		objectData.disabled = false; // デフォルトは有効（配置される）
	}

	// name を取得
	if (jsonNode.contains("name") && jsonNode["name"].is_string()) {
		objectData.name = jsonNode["name"];
	}

	if (jsonNode.contains("file_name")) {
		objectData.fileName = jsonNode["file_name"];
	}

	const auto& transform = jsonNode["transform"];
	objectData.translation.x = (float)transform["translation"][0];
	objectData.translation.y = (float)transform["translation"][2];
	objectData.translation.z = (float)transform["translation"][1];

	objectData.rotation.x = -(float)transform["rotation"][0];
	objectData.rotation.y = -(float)transform["rotation"][2];
	objectData.rotation.z = -(float)transform["rotation"][1];

	objectData.scaling.x = (float)transform["scale"][0];
	objectData.scaling.y = (float)transform["scale"][2];
	objectData.scaling.z = (float)transform["scale"][1];

	// 子オブジェクトも再帰処理
	if (jsonNode.contains("children")) {
		for (const auto& childJson : jsonNode["children"]) {
			LevelData::ObjectData child = ConvertJsonToObject(childJson);
			objectData.children.push_back(child);
		}
	}

	return objectData;
}

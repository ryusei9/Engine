#include "../util/JsonLoader.h"
#include <cmath> // 追加

constexpr float kDeg2Rad = 3.14159265358979323846f / 180.0f;

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
					playerData.rotation.x = -(float)transform["rotation"][0] * kDeg2Rad;
					playerData.rotation.y = -(float)transform["rotation"][2] * kDeg2Rad;
					playerData.rotation.z = -(float)transform["rotation"][1] * kDeg2Rad;
				}
				// プレイヤーのデータを追加
				levelData->players.push_back(playerData);
			} else if (objectJson.contains("type") && objectJson["type"] == "EnemySpawn") {
				LevelData::EnemyData enemyData;
				// 敵のファイル名を取得
				if (objectJson.contains("file_name") && objectJson["file_name"].is_string()) {
					enemyData.fileName = objectJson["file_name"];
				}
				// 敵の位置と回転を取得
				if (objectJson.contains("transform")) {
					const auto& transform = objectJson["transform"];
					enemyData.translation.x = (float)transform["translation"][0];
					enemyData.translation.y = (float)transform["translation"][2];
					enemyData.translation.z = (float)transform["translation"][1];
					enemyData.rotation.x = -(float)transform["rotation"][0] * kDeg2Rad;
					enemyData.rotation.y = -(float)transform["rotation"][2] * kDeg2Rad;
					enemyData.rotation.z = -(float)transform["rotation"][1] * kDeg2Rad;
				}
				// 敵のデータを追加
				levelData->enemies.push_back(enemyData);
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

	objectData.rotation.x = -(float)transform["rotation"][0] * kDeg2Rad;
	objectData.rotation.y = -(float)transform["rotation"][2] * kDeg2Rad;
	objectData.rotation.z = -(float)transform["rotation"][1] * kDeg2Rad;

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

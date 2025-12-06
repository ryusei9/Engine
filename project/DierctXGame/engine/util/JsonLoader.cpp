#include "../util/JsonLoader.h"
#include <cmath>
#include <fstream>
//#include <nlohmann/json.hpp>

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

    assert(deserialized.is_object());
    assert(deserialized.contains("name"));
    assert(deserialized["name"].is_string());

    std::string sceneName = deserialized["name"].get<std::string>();
    assert(sceneName == "scene");

    LevelData* levelData = new LevelData();

    if (deserialized.contains("objects") && deserialized["objects"].is_array()) {
        for (const auto& objectJson : deserialized["objects"]) {
            std::string type = objectJson.contains("type") ? objectJson["type"].get<std::string>() : "";

            if (objectJson.contains("type") && objectJson["type"] == "MESH") {

                LevelData::ObjectData obj = ConvertJsonToObject(objectJson);
                levelData->objects.push_back(obj);
            }
            else if (objectJson.contains("type") && objectJson["type"] == "CURVE") {
                LevelData::CurveData curveData;

                if (objectJson.contains("name")) {
                    curveData.fileName = objectJson["name"].get<std::string>();
                }

                float parentY = 0.0f;
                if (objectJson.contains("transform") && objectJson["transform"].contains("translation")) {
                    parentY = objectJson["transform"]["translation"][1].get<float>();
                }

                if (objectJson.contains("curve")) {
                    const auto& curve = objectJson["curve"];
                    for (const auto& spline : curve["splines"]) {
                        int index = 0;
                        for (const auto& point : spline["points"]) {
                            Vector3 v;
                            v.x = point["co"][0].get<float>();
                            v.y = point["co"][2].get<float>();
                            v.z = point["co"][1].get<float>() + parentY;

                            curveData.points.push_back(v);

                            // --- ★ time を読み込む ---
                            if (point.contains("time")) {
                                curveData.times.push_back(point["time"].get<float>());
                            }
                            else {
                                curveData.times.push_back(1.0f); // fallback
                            }

                            index++;
                        }
                    }
                }

                levelData->curves.push_back(curveData);
            }
            // PlayerSpawn/EnemySpawnは現状jsonに存在しないが、将来拡張用
            else if (objectJson.contains("type") && objectJson["type"] == "PlayerSpawn") {
                LevelData::PlayerData playerData;
                if (objectJson.contains("transform")) {
                    const auto& transform = objectJson["transform"];
                    playerData.translation.x = (float)transform["translation"][0];
                    playerData.translation.y = (float)transform["translation"][2];
                    playerData.translation.z = (float)transform["translation"][1];
                    playerData.rotation.x = -(float)transform["rotation"][0] * kDeg2Rad;
                    playerData.rotation.y = -(float)transform["rotation"][2] * kDeg2Rad;
                    playerData.rotation.z = -(float)transform["rotation"][1] * kDeg2Rad;
                }
                levelData->players.push_back(playerData);
            }
            else if (type == "EnemySpawn") {
                LevelData::EnemyData enemyData;
                if (objectJson.contains("File_name")) {
                    enemyData.fileName = objectJson["File_name"].get<std::string>();
                }
                if (objectJson.contains("transform")) {
                    const auto& transform = objectJson["transform"];
                    enemyData.translation.x = (float)transform["translation"][0];
                    enemyData.translation.y = (float)transform["translation"][2];
                    enemyData.translation.z = (float)transform["translation"][1];
                    enemyData.rotation.x = -(float)transform["rotation"][0] * kDeg2Rad;
                    enemyData.rotation.y = -(float)transform["rotation"][2] * kDeg2Rad;
                    enemyData.rotation.z = -(float)transform["rotation"][1] * kDeg2Rad;
                }
                levelData->enemies.push_back(enemyData);
            }
        }
    }

    return levelData;
}

LevelData::ObjectData JsonLoader::ConvertJsonToObject(const nlohmann::json& jsonNode)
{
    LevelData::ObjectData objectData;

    // name
    if (jsonNode.contains("name") && jsonNode["name"].is_string()) {
        objectData.name = jsonNode["name"].get<std::string>();
    }

    // fileName (MESHのみ)
    if (jsonNode.contains("File_name") && jsonNode["File_name"].is_string()) {
        objectData.fileName = jsonNode["File_name"].get<std::string>();
    }

    // transform
    if (jsonNode.contains("transform")) {
        const auto& transform = jsonNode["transform"];
        objectData.translation.x = (float)transform["translation"][0];
        objectData.translation.y = (float)transform["translation"][2];
        objectData.translation.z = (float)transform["translation"][1];

        objectData.rotation.x = (float)transform["rotation"][0] * kDeg2Rad;
        objectData.rotation.y = (float)transform["rotation"][2] * kDeg2Rad;
        objectData.rotation.z = (float)transform["rotation"][1] * kDeg2Rad;

        objectData.scaling.x = (float)transform["scale"][0];
        objectData.scaling.y = (float)transform["scale"][2];
        objectData.scaling.z = (float)transform["scale"][1];
    }

    // disabled
    objectData.disabled = (jsonNode.contains("disabled") && jsonNode["disabled"].is_boolean()) ? jsonNode["disabled"].get<bool>() : false;

    // children
    if (jsonNode.contains("children") && jsonNode["children"].is_array()) {
        for (const auto& childJson : jsonNode["children"]) {
            LevelData::ObjectData child = ConvertJsonToObject(childJson);
            objectData.children.push_back(child);
        }
    }

    return objectData;
}

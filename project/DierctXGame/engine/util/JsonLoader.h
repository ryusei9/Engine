#pragma once
#include <string>
#include <fstream>
#include <json.hpp>
#include <LevelData.h>

struct PlayerSpawnData {
	Vector3 translation; // プレイヤーの位置
    Vector3 rotation;    // プレイヤーの回転
};
struct EnemySpawnData {
    std::string fileName;   // ファイル名
	Vector3 translation; // 位置
    Vector3 rotation;   // 回転
};

struct CurveData {
    std::string fileName;
	std::vector<Vector3> points;
};
class JsonLoader {
public:
    // JSONファイルを読み込む
    static LevelData* Load(const std::string& fileName);

    // オブジェクトを走査するための再帰関数
    static LevelData::ObjectData ConvertJsonToObject(const nlohmann::json& jsonNode);

private:
    // 自キャラ配列
	std::vector<PlayerSpawnData> players;

    // 敵キャラ配列
    std::vector<EnemySpawnData> enemies;
};
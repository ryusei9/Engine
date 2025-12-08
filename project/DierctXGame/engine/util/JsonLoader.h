#pragma once
#include <string>
#include <fstream>
#include <json.hpp>
#include <LevelData.h>

// プレイヤースポーンデータ構造体
struct PlayerSpawnData {
	Vector3 translation; // プレイヤーの位置
    Vector3 rotation;    // プレイヤーの回転
};
// 敵スポーンデータ構造体
struct EnemySpawnData {
    std::string fileName;   // ファイル名
	Vector3 translation; // 位置
    Vector3 rotation;   // 回転
};

struct CurveData {
    std::string fileName;
	std::vector<Vector3> points;
    std::vector<float> times;
};

/// <summary>
/// Jsonファイルを読み込むクラス
/// </summary>
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
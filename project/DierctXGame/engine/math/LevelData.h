#pragma once
#include <string>
#include <vector>
#include "Vector3.h"

// 1つのオブジェクト情報
// レベルデータ構造体
struct LevelData {
	// オブジェクトデータ構造体
    struct ObjectData {
        std::string fileName;
        std::string name;
        std::vector<LevelData::ObjectData> children;
        Vector3 translation;
        Vector3 rotation;
        Vector3 scaling;
        bool disabled;
    };
    std::vector<ObjectData> objects;

	// プレイヤーデータ構造体
    struct PlayerData {
        Vector3 translation; // プレイヤーの位置
        Vector3 rotation;    // プレイヤーの回転
	};
	std::vector<PlayerData> players; // プレイヤーのデータ

	// 敵データ構造体
    struct EnemyData {
        std::string fileName;   // ファイル名
        Vector3 translation; // 位置
        Vector3 rotation;   // 回転
	};
	std::vector<EnemyData> enemies; // 敵のデータ

    struct CurveData {
        std::string fileName;
        std::vector<Vector3> points;
        std::vector<float> times;
    };
    std::vector<CurveData> curves;
};
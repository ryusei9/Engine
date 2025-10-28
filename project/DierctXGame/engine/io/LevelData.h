#pragma once
#include <string>
#include <vector>
#include "Vector3.h"

// 1つのオブジェクト情報

struct LevelData {
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

    struct PlayerData {
        Vector3 translation; // プレイヤーの位置
        Vector3 rotation;    // プレイヤーの回転
	};
	std::vector<PlayerData> players; // プレイヤーのデータ

    struct EnemyData {
        std::string fileName;   // ファイル名
        Vector3 translation; // 位置
        Vector3 rotation;   // 回転
	};
	std::vector<EnemyData> enemies; // 敵のデータ

    struct CurveData {
        std::string fileName;
        std::vector<Vector3> points;
    };
    std::vector<CurveData> curves;
};
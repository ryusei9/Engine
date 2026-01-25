#pragma once
#include <string>
#include <vector>
#include "Vector3.h"

// EnemyMove列挙型（グローバルスコープで定義）
enum class EnemyMove {
	None = 0,
	WaveMinusZ,
	WavePlusZ,
	WaveMinusY,
	WavePlusY
};

// カーブデータ構造体（共通定義）
struct CurveData {
    std::string fileName;
    std::vector<Vector3> points;
    std::vector<float> times;
};

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

    // ★ 敵は「アンカー」と「動きの種類」だけ持つ
    struct EnemyData {
        std::string fileName;
        Vector3 translation;   // ← 最終到達点（アンカー）
        Vector3 rotation;
        EnemyMove move;        // enumで指定
    };
    std::vector<EnemyData> enemies;

    // ★ カーブは共有
    std::vector<CurveData> curves;
};
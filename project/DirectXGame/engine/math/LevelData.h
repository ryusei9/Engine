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

/// <summary>
/// カーブデータ構造体（共通定義）
/// </summary>
struct CurveData {
	std::string fileName;
	std::vector<Vector3> points;
	std::vector<float> times;
};

/// <summary>
/// レベルデータ構造体
/// </summary>
struct LevelData {
	/// <summary>
	/// オブジェクトデータ構造体
	/// </summary>
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

	/// <summary>
	/// プレイヤーデータ構造体
	/// </summary>
	struct PlayerData {
		Vector3 translation; // プレイヤーの位置
		Vector3 rotation;    // プレイヤーの回転
	};
	std::vector<PlayerData> players; // プレイヤーのデータ

	/// <summary>
	/// 敵データ構造体（アンカーと動きの種類を持つ）
	/// </summary>
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
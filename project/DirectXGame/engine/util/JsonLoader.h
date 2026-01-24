#pragma once
#include <json.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include "LevelData.h" // CurveData と EnemyMove の定義を含む
#include "Vector3.h"

// ★ EnemyMove の定義を削除（LevelData.h に統一）

// 前方宣言
struct EnemyParameters;
struct EnemyAttackParameters;
struct EnemyBulletParameters;
struct PlayerParameters;
struct PlayerBulletParameters;
struct PlayerChargeBulletParameters;

// プレイヤースポーンデータ構造体
struct PlayerSpawnData {
	Vector3 translation;
	Vector3 rotation;
};

// 敵スポーンデータ構造体
struct EnemySpawnData {
	std::string fileName;
	Vector3 translation;
	Vector3 rotation;
};

// ステージデータ構造体（データテーブル用）
struct StageData {
	// 自キャラの発生データ
	PlayerSpawnData playerSpawnData;
	// 敵の発生データ
	std::vector<EnemySpawnData> enemySpawnDatas;
	// ステージの制限時間（秒）
	int timeLimit = 0;
};



/// <summary>
/// Jsonファイルを読み込むクラス
/// </summary>
class JsonLoader {
public:
	// JSONファイルを読み込む（レベルデータ用）
	static LevelData* Load(const std::string& fileName);

	// ステージデータをJSONファイルから読み込む
	static std::vector<StageData> LoadStageData(const std::string& fileName);

	// ステージデータを取得する（データ駆動設計）
	static StageData GetStageData(const std::vector<StageData>& stageDataTable, int stageIndex);

	// 敵のパラメータをJSONファイルから読み込む
	static EnemyParameters LoadEnemyParameters(const std::string& fileName);

	// 敵攻撃のパラメータをJSONファイルから読み込む
	static EnemyAttackParameters LoadEnemyAttackParameters(const std::string& fileName);

	// 敵の弾のパラメータをJSONファイルから読み込む
	static EnemyBulletParameters LoadEnemyBulletParameters(const std::string& fileName);

	// プレイヤーのパラメータをJSONファイルから読み込む
	static PlayerParameters LoadPlayerParameters(const std::string& fileName);

	// プレイヤー弾のパラメータをJSONファイルから読み込む
	static PlayerBulletParameters LoadPlayerBulletParameters(const std::string& fileName);

	// プレイヤーチャージ弾のパラメータをJSONファイルから読み込む
	static PlayerChargeBulletParameters LoadPlayerChargeBulletParameters(const std::string& fileName);

	// 敵のカーブデータをJSONファイルから読み込む
	static std::unordered_map<std::string, CurveData> LoadEnemyCurves(const std::string& fileName);

	// オブジェクトを走査するための再帰関数
	static LevelData::ObjectData ConvertJsonToObject(const nlohmann::json& jsonNode);

	// EnemyMove文字列をEnemyMove列挙型に変換する
	static EnemyMove ParseEnemyMove(const std::string& move);
};
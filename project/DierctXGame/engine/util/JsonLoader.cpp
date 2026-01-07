#include "../util/JsonLoader.h"
#include <cmath>
#include <fstream>
#include <cassert>
#include <Enemy.h> // EnemyParameters用
#include <EnemyAttack.h> // EnemyAttackParameters用
#include <EnemyBullet.h> // EnemyBulletParameters用
#include <Player.h> // PlayerParameters用
#include <PlayerBullet.h> // PlayerBulletParameters用
#include <PlayerChargeBullet.h> // PlayerChargeBulletParameters用

namespace {
	constexpr float kDeg2Rad = 3.14159265358979323846f / 180.0f;
	const std::string kDefaultBaseDirectory = "resources/";
	const std::string kExtension = ".json";
}

LevelData* JsonLoader::Load(const std::string& fileName)
{
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
							v.z = point["co"][1].get<float>() + parentY - 10.0f;

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
					enemyData.translation.z = (float)transform["translation"][1] + 10.0f;
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

std::vector<StageData> JsonLoader::LoadStageData(const std::string& fileName)
{
	const std::string fullpath = kDefaultBaseDirectory + fileName + kExtension;

	std::ifstream file(fullpath);
	if (file.fail()) {
		assert(!"ステージデータファイルを開けませんでした");
	}

	nlohmann::json deserialized;
	file >> deserialized;

	assert(deserialized.is_object());
	assert(deserialized.contains("stages"));
	assert(deserialized["stages"].is_array());

	std::vector<StageData> stageDataTable;

	for (const auto& stageJson : deserialized["stages"]) {
		StageData stageData;

		// プレイヤースポーンデータの読み込み
		if (stageJson.contains("playerSpawn")) {
			const auto& playerSpawn = stageJson["playerSpawn"];
			if (playerSpawn.contains("translation")) {
				stageData.playerSpawnData.translation.x = playerSpawn["translation"][0].get<float>();
				stageData.playerSpawnData.translation.y = playerSpawn["translation"][1].get<float>();
				stageData.playerSpawnData.translation.z = playerSpawn["translation"][2].get<float>();
			}
			if (playerSpawn.contains("rotation")) {
				stageData.playerSpawnData.rotation.x = playerSpawn["rotation"][0].get<float>() * kDeg2Rad;
				stageData.playerSpawnData.rotation.y = playerSpawn["rotation"][1].get<float>() * kDeg2Rad;
				stageData.playerSpawnData.rotation.z = playerSpawn["rotation"][2].get<float>() * kDeg2Rad;
			}
		}

		// 敵スポーンデータの読み込み
		if (stageJson.contains("enemySpawns") && stageJson["enemySpawns"].is_array()) {
			for (const auto& enemyJson : stageJson["enemySpawns"]) {
				EnemySpawnData enemyData;
				if (enemyJson.contains("fileName")) {
					enemyData.fileName = enemyJson["fileName"].get<std::string>();
				}
				if (enemyJson.contains("translation")) {
					enemyData.translation.x = enemyJson["translation"][0].get<float>();
					enemyData.translation.y = enemyJson["translation"][1].get<float>();
					enemyData.translation.z = enemyJson["translation"][2].get<float>();
				}
				if (enemyJson.contains("rotation")) {
					enemyData.rotation.x = enemyJson["rotation"][0].get<float>() * kDeg2Rad;
					enemyData.rotation.y = enemyJson["rotation"][1].get<float>() * kDeg2Rad;
					enemyData.rotation.z = enemyJson["rotation"][2].get<float>() * kDeg2Rad;
				}
				stageData.enemySpawnDatas.push_back(enemyData);
			}
		}

		// 時間制限の読み込み
		if (stageJson.contains("timeLimit")) {
			stageData.timeLimit = stageJson["timeLimit"].get<int>();
		}

		stageDataTable.push_back(stageData);
	}

	return stageDataTable;
}

StageData JsonLoader::GetStageData(const std::vector<StageData>& stageDataTable, int stageIndex)
{
	// ステージ番号が正常であることを保証する
	assert(0 <= stageIndex && stageIndex < static_cast<int>(stageDataTable.size()));
	// 指定ステージ番号のデータをテーブルから取得する
	return stageDataTable[stageIndex];
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

EnemyParameters JsonLoader::LoadEnemyParameters(const std::string& fileName)
{
	const std::string fullpath = kDefaultBaseDirectory + fileName + kExtension;

	std::ifstream file(fullpath);
	if (file.fail()) {
		// ファイルが開けない場合はデフォルト値を返す
		return EnemyParameters();
	}

	nlohmann::json deserialized;
	file >> deserialized;

	EnemyParameters params;

	// 基本パラメータ
	if (deserialized.contains("moveSpeed")) {
		params.moveSpeed = deserialized["moveSpeed"].get<float>();
	}
	if (deserialized.contains("initialHp")) {
		params.initialHp = deserialized["initialHp"].get<int32_t>();
	}
	if (deserialized.contains("respawnTimeSec")) {
		params.respawnTimeSec = deserialized["respawnTimeSec"].get<float>();
	}
	if (deserialized.contains("colliderRadius")) {
		params.colliderRadius = deserialized["colliderRadius"].get<float>();
	}
	if (deserialized.contains("shotIntervalSec")) {
		params.shotIntervalSec = deserialized["shotIntervalSec"].get<float>();
	}

	// 死亡演出パラメータ
	if (deserialized.contains("deathDurationSec")) {
		params.deathDurationSec = deserialized["deathDurationSec"].get<float>();
	}
	if (deserialized.contains("fallSpeed")) {
		params.fallSpeed = deserialized["fallSpeed"].get<float>();
	}
	if (deserialized.contains("rotationSpeed")) {
		params.rotationSpeed = deserialized["rotationSpeed"].get<float>();
	}

	// パーティクルパラメータ
	if (deserialized.contains("smokeParticleRate")) {
		params.smokeParticleRate = deserialized["smokeParticleRate"].get<int>();
	}
	if (deserialized.contains("smokeParticleCount")) {
		params.smokeParticleCount = deserialized["smokeParticleCount"].get<int>();
	}
	if (deserialized.contains("deathParticleRate")) {
		params.deathParticleRate = deserialized["deathParticleRate"].get<int>();
	}
	if (deserialized.contains("deathParticleCount")) {
		params.deathParticleCount = deserialized["deathParticleCount"].get<int>();
	}
	if (deserialized.contains("hitParticleRate")) {
		params.hitParticleRate = deserialized["hitParticleRate"].get<int>();
	}
	if (deserialized.contains("hitParticleCount")) {
		params.hitParticleCount = deserialized["hitParticleCount"].get<int>();
	}

	// 煙エフェクトパラメータ
	if (deserialized.contains("smokePower")) {
		params.smokePower = deserialized["smokePower"].get<float>();
	}
	if (deserialized.contains("smokePowerMultiplier")) {
		params.smokePowerMultiplier = deserialized["smokePowerMultiplier"].get<float>();
	}
	if (deserialized.contains("smokeUpwardForce")) {
		params.smokeUpwardForce = deserialized["smokeUpwardForce"].get<float>();
	}

	return params;
}

EnemyAttackParameters JsonLoader::LoadEnemyAttackParameters(const std::string& fileName)
{
	const std::string fullpath = kDefaultBaseDirectory + fileName + kExtension;

	std::ifstream file(fullpath);
	if (file.fail()) {
		// ファイルが開けない場合はデフォルト値を返す
		return EnemyAttackParameters();
	}

	nlohmann::json deserialized;
	file >> deserialized;

	EnemyAttackParameters params;

	// Pattern1: 扇形
	if (deserialized.contains("fanMoveSpeedY")) {
		params.fanMoveSpeedY = deserialized["fanMoveSpeedY"].get<float>();
	}
	if (deserialized.contains("fanMoveRangeY")) {
		params.fanMoveRangeY = deserialized["fanMoveRangeY"].get<float>();
	}
	if (deserialized.contains("fanShotIntervalSec")) {
		params.fanShotIntervalSec = deserialized["fanShotIntervalSec"].get<float>();
	}
	if (deserialized.contains("fanShotCount")) {
		params.fanShotCount = deserialized["fanShotCount"].get<int32_t>();
	}
	if (deserialized.contains("fanBaseAngle")) {
		params.fanBaseAngle = deserialized["fanBaseAngle"].get<float>();
	}
	if (deserialized.contains("fanSpread")) {
		params.fanSpread = deserialized["fanSpread"].get<float>();
	}
	if (deserialized.contains("fanBulletSpeed")) {
		params.fanBulletSpeed = deserialized["fanBulletSpeed"].get<float>();
	}

	// Pattern2: 自機狙い
	if (deserialized.contains("aimedShotIntervalSec")) {
		params.aimedShotIntervalSec = deserialized["aimedShotIntervalSec"].get<float>();
	}
	if (deserialized.contains("aimedBulletSpeed")) {
		params.aimedBulletSpeed = deserialized["aimedBulletSpeed"].get<float>();
	}
	if (deserialized.contains("bulletSpeedScale")) {
		params.bulletSpeedScale = deserialized["bulletSpeedScale"].get<float>();
	}
	if (deserialized.contains("aimedMinLen")) {
		params.aimedMinLen = deserialized["aimedMinLen"].get<float>();
	}

	// Pattern3: 突進＋全方位
	if (deserialized.contains("rushStartX")) {
		params.rushStartX = deserialized["rushStartX"].get<float>();
	}
	if (deserialized.contains("rushSpeedX")) {
		params.rushSpeedX = deserialized["rushSpeedX"].get<float>();
	}
	if (deserialized.contains("rushShotIntervalSec")) {
		params.rushShotIntervalSec = deserialized["rushShotIntervalSec"].get<float>();
	}
	if (deserialized.contains("rushRingCount")) {
		params.rushRingCount = deserialized["rushRingCount"].get<int32_t>();
	}
	if (deserialized.contains("rushRingSpeed")) {
		params.rushRingSpeed = deserialized["rushRingSpeed"].get<float>();
	}
	if (deserialized.contains("rushLeftEndX")) {
		params.rushLeftEndX = deserialized["rushLeftEndX"].get<float>();
	}
	if (deserialized.contains("rushRespawnX")) {
		params.rushRespawnX = deserialized["rushRespawnX"].get<float>();
	}
	if (deserialized.contains("rushResetX")) {
		params.rushResetX = deserialized["rushResetX"].get<float>();
	}

	// Pattern遷移
	if (deserialized.contains("pattern1DurationSec")) {
		params.pattern1DurationSec = deserialized["pattern1DurationSec"].get<float>();
	}

	// Pattern4: 待機
	if (deserialized.contains("waitEasingDurationSec")) {
		params.waitEasingDurationSec = deserialized["waitEasingDurationSec"].get<float>();
	}

	return params;
}

EnemyBulletParameters JsonLoader::LoadEnemyBulletParameters(const std::string& fileName)
{
	const std::string fullpath = kDefaultBaseDirectory + fileName + kExtension;

	std::ifstream file(fullpath);
	if (file.fail()) {
		// ファイルが開けない場合はデフォルト値を返す
		return EnemyBulletParameters();
	}

	nlohmann::json deserialized;
	file >> deserialized;

	EnemyBulletParameters params;

	// 生存フレーム数
	if (deserialized.contains("lifeFrames")) {
		params.lifeFrames = deserialized["lifeFrames"].get<uint32_t>();
	}

	// 当たり判定半径
	if (deserialized.contains("radius")) {
		params.radius = deserialized["radius"].get<float>();
	}

	// 初期スケール
	if (deserialized.contains("initScale")) {
		params.initScale.x = deserialized["initScale"][0].get<float>();
		params.initScale.y = deserialized["initScale"][1].get<float>();
		params.initScale.z = deserialized["initScale"][2].get<float>();
	}

	// 初期回転
	if (deserialized.contains("initRotate")) {
		params.initRotate.x = deserialized["initRotate"][0].get<float>();
		params.initRotate.y = deserialized["initRotate"][1].get<float>();
		params.initRotate.z = deserialized["initRotate"][2].get<float>();
	}

	// モデルファイル名
	if (deserialized.contains("modelFileName")) {
		params.modelFileName = deserialized["modelFileName"].get<std::string>();
	}

	return params;
}

PlayerParameters JsonLoader::LoadPlayerParameters(const std::string& fileName)
{
	const std::string fullpath = kDefaultBaseDirectory + fileName + kExtension;

	std::ifstream file(fullpath);
	if (file.fail()) {
		// ファイルが開けない場合はデフォルト値を返す
		return PlayerParameters();
	}

	nlohmann::json deserialized;
	file >> deserialized;

	PlayerParameters params;

	// 基本パラメータ
	if (deserialized.contains("moveSpeed")) {
		params.moveSpeed = deserialized["moveSpeed"].get<float>();
	}
	if (deserialized.contains("radius")) {
		params.radius = deserialized["radius"].get<float>();
	}
	if (deserialized.contains("respawnWaitSec")) {
		params.respawnWaitSec = deserialized["respawnWaitSec"].get<float>();
	}

	// チャージ関連
	if (deserialized.contains("chargeReadySec")) {
		params.chargeReadySec = deserialized["chargeReadySec"].get<float>();
	}

	// パーティクルパラメータ
	if (deserialized.contains("thrusterRate")) {
		params.thrusterRate = deserialized["thrusterRate"].get<uint32_t>();
	}
	if (deserialized.contains("thrusterCount")) {
		params.thrusterCount = deserialized["thrusterCount"].get<uint32_t>();
	}
	if (deserialized.contains("thrusterOffsetX")) {
		params.thrusterOffsetX = deserialized["thrusterOffsetX"].get<float>();
	}
	if (deserialized.contains("explosionRate")) {
		params.explosionRate = deserialized["explosionRate"].get<float>();
	}
	if (deserialized.contains("explosionCount")) {
		params.explosionCount = deserialized["explosionCount"].get<uint32_t>();
	}

	// スラスター計算パラメータ
	if (deserialized.contains("thrusterBasePower")) {
		params.thrusterBasePower = deserialized["thrusterBasePower"].get<float>();
	}
	if (deserialized.contains("thrusterVelocityMultiplier")) {
		params.thrusterVelocityMultiplier = deserialized["thrusterVelocityMultiplier"].get<float>();
	}

	// 初期Transform
	if (deserialized.contains("initScale")) {
		params.initScale.x = deserialized["initScale"][0].get<float>();
		params.initScale.y = deserialized["initScale"][1].get<float>();
		params.initScale.z = deserialized["initScale"][2].get<float>();
	}
	if (deserialized.contains("initRotate")) {
		params.initRotate.x = deserialized["initRotate"][0].get<float>();
		params.initRotate.y = deserialized["initRotate"][1].get<float>();
		params.initRotate.z = deserialized["initRotate"][2].get<float>();
	}
	if (deserialized.contains("initTranslate")) {
		params.initTranslate.x = deserialized["initTranslate"][0].get<float>();
		params.initTranslate.y = deserialized["initTranslate"][1].get<float>();
		params.initTranslate.z = deserialized["initTranslate"][2].get<float>();
	}

	// モデルファイル名
	if (deserialized.contains("modelFileName")) {
		params.modelFileName = deserialized["modelFileName"].get<std::string>();
	}

	// パーティクルテクスチャファイル名
	if (deserialized.contains("thrusterTexture")) {
		params.thrusterTexture = deserialized["thrusterTexture"].get<std::string>();
	}
	if (deserialized.contains("explosionTexture")) {
		params.explosionTexture = deserialized["explosionTexture"].get<std::string>();
	}

	return params;
}

PlayerBulletParameters JsonLoader::LoadPlayerBulletParameters(const std::string& fileName)
{
	const std::string fullpath = kDefaultBaseDirectory + fileName + kExtension;

	std::ifstream file(fullpath);
	if (file.fail()) {
		// ファイルが開けない場合はデフォルト値を返す
		return PlayerBulletParameters();
	}

	nlohmann::json deserialized;
	file >> deserialized;

	PlayerBulletParameters params;

	// 生存フレーム
	if (deserialized.contains("lifeFrames")) {
		params.lifeFrames = deserialized["lifeFrames"].get<uint32_t>();
	}

	// 移動速度
	if (deserialized.contains("speed")) {
		params.speed = deserialized["speed"].get<float>();
	}

	// 当たり半径
	if (deserialized.contains("radius")) {
		params.radius = deserialized["radius"].get<float>();
	}

	// 初期スケール
	if (deserialized.contains("initScale")) {
		params.initScale.x = deserialized["initScale"][0].get<float>();
		params.initScale.y = deserialized["initScale"][1].get<float>();
		params.initScale.z = deserialized["initScale"][2].get<float>();
	}

	// 初期回転
	if (deserialized.contains("initRotate")) {
		params.initRotate.x = deserialized["initRotate"][0].get<float>();
		params.initRotate.y = deserialized["initRotate"][1].get<float>();
		params.initRotate.z = deserialized["initRotate"][2].get<float>();
	}

	// 速度ベクトル方向
	if (deserialized.contains("velocityDirection")) {
		params.velocityDirection.x = deserialized["velocityDirection"][0].get<float>();
		params.velocityDirection.y = deserialized["velocityDirection"][1].get<float>();
		params.velocityDirection.z = deserialized["velocityDirection"][2].get<float>();
	}

	// モデルファイル名
	if (deserialized.contains("modelFileName")) {
		params.modelFileName = deserialized["modelFileName"].get<std::string>();
	}

	return params;
}

PlayerChargeBulletParameters JsonLoader::LoadPlayerChargeBulletParameters(const std::string& fileName)
{
	const std::string fullpath = kDefaultBaseDirectory + fileName + kExtension;

	std::ifstream file(fullpath);
	if (file.fail()) {
		// ファイルが開けない場合はデフォルト値を返す
		return PlayerChargeBulletParameters();
	}

	nlohmann::json deserialized;
	file >> deserialized;

	PlayerChargeBulletParameters params;

	// チャージ弾専用パラメータ
	if (deserialized.contains("damage")) {
		params.damage = deserialized["damage"].get<float>();
	}
	if (deserialized.contains("radius")) {
		params.radius = deserialized["radius"].get<float>();
	}
	if (deserialized.contains("scaleFactor")) {
		params.scaleFactor = deserialized["scaleFactor"].get<float>();
	}
	if (deserialized.contains("serialStart")) {
		params.serialStart = deserialized["serialStart"].get<uint32_t>();
	}

	// 基底クラス（PlayerBullet）のパラメータ
	if (deserialized.contains("baseBullet")) {
		const auto& baseBullet = deserialized["baseBullet"];
		
		if (baseBullet.contains("lifeFrames")) {
			params.baseBulletParams.lifeFrames = baseBullet["lifeFrames"].get<uint32_t>();
		}
		if (baseBullet.contains("speed")) {
			params.baseBulletParams.speed = baseBullet["speed"].get<float>();
		}
		if (baseBullet.contains("radius")) {
			params.baseBulletParams.radius = baseBullet["radius"].get<float>();
		}
		if (baseBullet.contains("initScale")) {
			params.baseBulletParams.initScale.x = baseBullet["initScale"][0].get<float>();
			params.baseBulletParams.initScale.y = baseBullet["initScale"][1].get<float>();
			params.baseBulletParams.initScale.z = baseBullet["initScale"][2].get<float>();
		}
		if (baseBullet.contains("initRotate")) {
			params.baseBulletParams.initRotate.x = baseBullet["initRotate"][0].get<float>();
			params.baseBulletParams.initRotate.y = baseBullet["initRotate"][1].get<float>();
			params.baseBulletParams.initRotate.z = baseBullet["initRotate"][2].get<float>();
		}
		if (baseBullet.contains("velocityDirection")) {
			params.baseBulletParams.velocityDirection.x = baseBullet["velocityDirection"][0].get<float>();
			params.baseBulletParams.velocityDirection.y = baseBullet["velocityDirection"][1].get<float>();
			params.baseBulletParams.velocityDirection.z = baseBullet["velocityDirection"][2].get<float>();
		}
		if (baseBullet.contains("modelFileName")) {
			params.baseBulletParams.modelFileName = baseBullet["modelFileName"].get<std::string>();
		}
	}

	return params;
}

#pragma once
#include <string>
#include <fstream>
#include <json.hpp>
#include <LevelData.h>

class JsonLoader {
public:
    // JSONファイルを読み込む
    static LevelData* Load(const std::string& fileName);

    // オブジェクトを走査するための再帰関数
    static LevelData::ObjectData ConvertJsonToObject(const nlohmann::json& jsonNode);
};
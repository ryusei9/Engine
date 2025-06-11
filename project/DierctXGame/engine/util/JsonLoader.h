#pragma once
#include <string>
#include <fstream>
#include <json.hpp>
#include <LevelData.h>

class JsonLoader {
public:
    // JSONファイルを読み込む
    static LevelData* Load(const std::string& fileName);
};
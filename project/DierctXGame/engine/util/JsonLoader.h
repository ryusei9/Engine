#pragma once
#include <string>
#include <fstream>
#include <json.hpp>

class JsonLoader {
public:
    // JSONファイルを読み込む
    static nlohmann::json Load(const std::string& fileName);
};
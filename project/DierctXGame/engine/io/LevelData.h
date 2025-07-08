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
};
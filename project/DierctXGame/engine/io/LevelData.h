#pragma once
#include <string>
#include <vector>
#include "Vector3.h"

// 1つのオブジェクト情報

struct LevelData {
    struct ObjectData {
        std::string fileName;
        Vector3 translation;
        Vector3 rotation;
        Vector3 scaling;
    };
    std::vector<ObjectData> objects;
};
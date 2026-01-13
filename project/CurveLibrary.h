#pragma once
#include <unordered_map>
#include <string>
#include "Enemy.h" // EnemyMove用
#include "LevelData.h" // CurveData用

class CurveLibrary
{
public:
    static void Initialize(const std::vector<CurveData>& curves);

    static const CurveData& Get(const std::string& name);

private:
    static std::unordered_map<std::string, CurveData> curves_;
};

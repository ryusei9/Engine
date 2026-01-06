#pragma once
#include <unordered_map>
#include <string>
#include "Enemy.h" // EnemyMove用
#include "LevelData.h" // CurveData用

class CurveLibrary
{
public:
    static void Load(const std::string& filePath);

    static const CurveData* GetCurve(EnemyMove type);

private:
    static std::unordered_map<EnemyMove, CurveData> curves_;
};

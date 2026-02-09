#pragma once
#include <unordered_map>
#include <string>
#include "Enemy.h" // EnemyMove用
#include "LevelData.h" // CurveData用
namespace MyEngine {
    class CurveLibrary
    {
    public:
        static void Register(EnemyMove move, const CurveData& curve);

        static const CurveData& Get(EnemyMove type);

        const CurveData* TryGet(EnemyMove type);

        static void Clear() { curves_.clear(); }

    private:
        static std::unordered_map<EnemyMove, CurveData> curves_;
    };
}
#include "CurveLibrary.h"
#include "JsonLoader.h"

std::unordered_map<EnemyMove, CurveData> CurveLibrary::curves_;

void CurveLibrary::Load(const std::string& filePath)
{
    curves_.clear();

    // JSONからカーブデータを名前ベースで読み込む
    auto curveMap = JsonLoader::LoadEnemyCurves(filePath);

    // Enemy_Wave_-Z が存在するかチェック
    if (curveMap.find("Enemy_Wave_-Z") != curveMap.end()) {
        curves_[EnemyMove::WaveMinusZ] = curveMap["Enemy_Wave_-Z"];
    }else {
        curves_[EnemyMove::WavePlusZ] = curveMap["Enemy_Wave_+Z"];
    }
}

const CurveData* CurveLibrary::GetCurve(EnemyMove type)
{
    auto it = curves_.find(type);
    if (it == curves_.end()) {
        return nullptr;
    }
    return &it->second;
}

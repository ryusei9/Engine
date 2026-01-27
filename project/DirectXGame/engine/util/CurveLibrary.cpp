#include "CurveLibrary.h"
#include "JsonLoader.h"

std::unordered_map<EnemyMove, CurveData> CurveLibrary::curves_;

void CurveLibrary::Register(EnemyMove type, const CurveData& curve)
{
    curves_[type] = curve;
}

const CurveData& CurveLibrary::Get(EnemyMove type) {
    auto it = curves_.find(type);
    assert(it != curves_.end() && "Curve not found for EnemyMove");
    return it->second;
}

const CurveData* CurveLibrary::TryGet(EnemyMove type)
{
    auto it = curves_.find(type);
    if (it == curves_.end()) {
        return nullptr;
    }
    return &it->second;
}

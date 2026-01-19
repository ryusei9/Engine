#include "CurveLibrary.h"
#include "JsonLoader.h"

std::unordered_map<EnemyMove, CurveData> CurveLibrary::curves_;

void CurveLibrary::Initialize(
	const std::vector<CurveData>& curves)
{
    curves_.clear();

    for (const auto& curve : curves) {
        if (curve.fileName == "Enemy_Wave_-Z") {
            curves_[EnemyMove::WaveMinusZ] = curve;
        }
        else if (curve.fileName == "Enemy_Wave_+Z") {
            curves_[EnemyMove::WavePlusZ] = curve;
        }
    }
}

const CurveData& CurveLibrary::Get(EnemyMove type) {
    auto it = curves_.find(type);
    assert(it != curves_.end() && "Curve not found for EnemyMove");
    return it->second;
}

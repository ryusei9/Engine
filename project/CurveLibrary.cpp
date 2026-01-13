#include "CurveLibrary.h"
#include "JsonLoader.h"

std::unordered_map<std::string, CurveData> CurveLibrary::curves_;

void CurveLibrary::Initialize(const std::vector<CurveData>& curves)
{
    curves_.clear();
    for (const auto& c : curves) {
        curves_[c.fileName] = c;
    }
}

const CurveData& CurveLibrary::Get(const std::string& name)
{
    auto it = curves_.find(name);
    assert(it != curves_.end() && "Curve not found");
    return it->second;
}

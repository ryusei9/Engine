#include "CurveMoveManager.h"

void CurveMoveManager::Start(const CurveData& curve)
{
    curve_ = curve;
    currentIndex_ = 0;
    timer_ = 0.0f;
    finished_ = false;

    
    position_ = curve_.points[0];
    
    // ★ 防御
    assert(curve_.points.size() == curve_.times.size());

    if (!curve_.points.empty()) {
        position_ = curve_.points[0];
    }
}

void CurveMoveManager::Update(float deltaTime)
{
    if (finished_) return;

    if (currentIndex_ >= curve_.points.size() - 1) {
        finished_ = true;
        return;
    }

    timer_ += deltaTime;

    float duration = curve_.times[currentIndex_ + 1];
    if (duration <= 0.0f) duration = 0.0001f;

    float t = timer_ / duration;

    if (t >= 1.0f) {
        currentIndex_++;
        timer_ = 0.0f;
        return;
    }

    uint32_t i0 = ClampIndex(currentIndex_ - 1);
    uint32_t i1 = ClampIndex(currentIndex_);
    uint32_t i2 = ClampIndex(currentIndex_ + 1);
    uint32_t i3 = ClampIndex(currentIndex_ + 2);

    Vector3 p = CatmullRom(
        curve_.points[i0],
        curve_.points[i1],
        curve_.points[i2],
        curve_.points[i3],
        t
    );


    position_ = p;
}

uint32_t CurveMoveManager::ClampIndex(uint32_t i) const
{
	// インデックスのクランプ
    if (i < 0) return 0;
    uint32_t max = static_cast<uint32_t>(curve_.points.size()) - 1;
    if (i > max) return max;
    return i;
}

#pragma once
#include <chrono>
// アプリ起動時の基準時刻を保持
static const auto gStartTime = std::chrono::steady_clock::now();

float GetNowTimeInSeconds() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::duration<float>>(now - gStartTime);
    return elapsed.count();
}
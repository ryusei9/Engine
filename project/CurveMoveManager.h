#pragma once
#include <LevelData.h>
#include <JsonLoader.h>
#include <Vector3.h>
#include <Lerp.h>

/// <summary>
/// カーブ移動管理クラス
/// </summary>
class CurveMoveManager
{
public:
    /*------メンバ関数------*/

	// カーブ移動開始
    void Start(const CurveData& curve);

	// カーブ移動更新
    void Update(float deltaTime);

    /*------ゲッター------*/

    // 終わりの取得
    bool IsFinished() const { return finished_; }

	// 位置の取得
    Vector3 GetPosition() const { return position_; }
private:
	// インデックスのクランプ
    uint32_t ClampIndex(uint32_t i) const;
private:
    /*------メンバ変数------*/

	// カーブデータ
    const CurveData* curve_ = nullptr;

	// 現在のインデックス
    uint32_t currentIndex_ = 0;

	// タイマー
    float timer_ = 0.0f;

	// 終了フラグ
    bool finished_ = false;

	// 現在の位置
    Vector3 position_;
};


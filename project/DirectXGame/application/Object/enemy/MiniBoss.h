#pragma once
#include "BaseCharacter.h"
#include <memory>

/// <summary>
/// 中ボスキャラクタークラス
/// </summary>
class MiniBoss : public BaseCharacter
{
public:

    /// <summary>
    /// ボスの状態
    /// </summary>
    enum class State
    {
        Alive,  // 生きている
        Dead    // 死亡
    };

public:

    /// <summary>
    /// コンストラクタ
    /// </summary>
    MiniBoss();

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update() override;

    /// <summary>
    /// 描画
    /// </summary>
    void Draw() override;

    /// <summary>
    /// 移動処理
    /// </summary>
    void Move() override;

    /// <summary>
    /// 攻撃処理
    /// </summary>
    void Attack() override;

    /// <summary>
    /// 衝突発生時の処理
    /// </summary>
    /// <param name="other">衝突した他のコライダー</param>
    void OnCollision(Collider* other) override;

    /// <summary>
    /// 中心座標の取得
    /// </summary>
    Vector3 GetCenterPosition() const override;

    // Getter
    /// <summary>
    /// HPの取得
    /// </summary>
    int GetHp() const { return hp_; }

    /// <summary>
    /// 半径の取得
    /// </summary>
    float GetRadius() const { return radius_; }

    /// <summary>
    /// 状態の取得
    /// </summary>
    State GetState() const { return state_; }

    // Setter
    /// <summary>
    /// HPの設定
    /// </summary>
    void SetHp(int hp) { hp_ = hp; }

    /// <summary>
    /// 半径の設定
    /// </summary>
    void SetRadius(float radius) { radius_ = radius; }

    /// <summary>
    /// 状態の設定
    /// </summary>
    void SetState(State state) { state_ = state; }

private:

	// ワールド変換の参照
    WorldTransform& worldTransform;

    // HP
    int hp_ = 100;

    // 半径
    float radius_ = 3.0f;

    // 状態
    State state_ = State::Alive;
};
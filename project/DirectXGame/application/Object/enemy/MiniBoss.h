#pragma once
#include "BaseCharacter.h"
#include <memory>

class MiniBoss : public BaseCharacter
{
public:

    enum class State
    {
        Alive,
        Dead
    };

public:

    // コンストラクタ
    MiniBoss();

    // 初期化
    void Initialize() override;

    // 更新
    void Update() override;

    // 描画
    void Draw() override;

    // 移動（後で実装）
    void Move() override;

    // 攻撃（後で実装）
    void Attack() override;

    // 衝突
    void OnCollision(Collider* other) override;

    // 中心座標
    Vector3 GetCenterPosition() const override;

    // Getter
    int GetHp() const { return hp_; }
    float GetRadius() const { return radius_; }
    State GetState() const { return state_; }

    // Setter
    void SetHp(int hp) { hp_ = hp; }
    void SetRadius(float radius) { radius_ = radius; }
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
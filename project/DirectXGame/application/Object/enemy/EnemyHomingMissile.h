#pragma once
#include "EnemyBullet.h"
#include "Player.h"

/// <summary>
/// 敵の追尾ミサイルクラス
/// </summary>
class EnemyHomingMissile : public EnemyBullet
{
public:

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="pos">初期位置</param>
    /// <param name="velocity">初速度</param>
    /// <param name="player">追尾対象となるプレイヤーパーティへのポインタ</param>
    /// <param name="paramFile">パラメータファイル名</param>
    void Initialize(
        const Vector3& pos,
        const Vector3& velocity,
        Player* player,
        const std::string& paramFile);

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update() override;

    /// <summary>
    /// 衝突発生時のコールバック
    /// </summary>
    /// <param name="other">衝突した他のコライダー</param>
    void OnCollision(Collider* other) override;

    // Getter
    /// <summary>
    /// プレイヤーの取得
    /// </summary>
    Player* GetPlayer() const { return player_; }

    /// <summary>
    /// 追尾時間の取得
    /// </summary>
    float GetHomingTime() const { return homingTime_; }

    /// <summary>
    /// 経過時間の取得
    /// </summary>
    float GetTimer() const { return timer_; }

    /// <summary>
    /// 旋回速度の取得
    /// </summary>
    float GetRotateSpeed() const { return rotateSpeed_; }

    /// <summary>
    /// 移動速度の取得
    /// </summary>
    float GetSpeed() const { return speed_; }

    // Setter
    /// <summary>
    /// プレイヤーの設定
    /// </summary>
    void SetPlayer(Player* player) { player_ = player; }
    /// <summary>
    /// 追尾時間の設定
    /// </summary>
    void SetHomingTime(float homingTime) { homingTime_ = homingTime; }
    /// <summary>
    /// 経過時間の設定
    /// </summary>
    void SetTimer(float timer) { timer_ = timer; }
    /// <summary>
    /// 旋回速度の設定
    /// </summary>
    void SetRotateSpeed(float rotateSpeed) { rotateSpeed_ = rotateSpeed; }
    /// <summary>
    /// 移動速度の設定
    /// </summary>
    void SetSpeed(float speed) { speed_ = speed; }

private:

    Player* player_ = nullptr;

    float homingTime_ = 3.0f;   // 追尾時間
    float timer_ = 0.0f;

    float rotateSpeed_ = 0.05f; // 旋回力

    float speed_ = 0.03f; // 好きな速さに調整
};
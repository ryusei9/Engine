#include "MiniBoss.h"
#include <Object3dCommon.h>
#include <CollisionTypeIdDef.h>
#include <PlayerBullet.h>
#include <PlayerChargeBullet.h>

using namespace MyEngine;

MiniBoss::MiniBoss()
    : worldTransform(BaseCharacter::GetWorldTransform())
{
    // シリアルナンバーを振る
    serialNumber_ = sNextSerialNumber_;
    // 次のシリアルナンバーに1を足す
	++sNextSerialNumber_;
}

void MiniBoss::Initialize()
{
	// 基底クラスの初期化関数を呼び出す
	BaseCharacter::Initialize();

	// ワールド変換情報の初期化
	GetWorldTransform().Initialize();
	// ボスキャラクターとしてのスケールサイズを設定
	SetScale({ 3.0f,3.0f,3.0f });
	// ボスキャラクターの初期位置（奥の方）を設定
	SetPosition({ 0.0f,0.0f,30.0f });

	// 描画用3Dモデルの生成と初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize("miniBoss.obj");

	// ベースカラーとして赤色を設定（分かりやすく強調）
	object3d_->SetMaterialColor({ 1.0f, 0.0f, 0.0f, 1.0f });

	// コライダーの当たり判定の半径を設定
	SetRadius(radius_);
	// コライダーに敵用のIDをセット
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kEnemy));

	// 初期の耐久力(HP)を設定
	hp_ = 100;
}

void MiniBoss::Update()
{
    // 状態が死亡の場合は更新処理を行わない
    if (state_ == State::Dead) {
        return;
    }

    // ボスの移動処理を実行
    Move();
    // ボスの攻撃処理を実行
    Attack();

    // 基底クラスの汎用更新処理を実行
    BaseCharacter::Update();

    // HPが0以下に達したかチェック
    if (hp_ <= 0) {
        // 状態を死亡に変更
        state_ = State::Dead;
        // 生存フラグを落としてゲームから除外
        SetIsAlive(false);
    }
}

void MiniBoss::Draw()
{
    // 生きている状態のみ描画を行う
    if (state_ == State::Alive) {
        BaseCharacter::Draw();
    }
}

void MiniBoss::Move()
{
    // 後で実装
}

void MiniBoss::Attack()
{
    // 後で実装
}

void MiniBoss::OnCollision(Collider* other)
{
    // 死んでいる場合は当たり判定を処理しない
    if (state_ != State::Alive) {
        return;
    }

    // 衝突相手がプレイヤーの通常弾か判定
    if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeIdDef::kPlayerBullet)) {
        // 通常弾の場合は小ダメージを与える
        hp_ -= 1;
    }

    // 衝突相手がプレイヤーのチャージ弾か判定
    if (other->GetTypeID() == static_cast<uint32_t>(CollisionTypeIdDef::kPlayerChargeBullet)) {
        // チャージ弾の場合は大ダメージを与える
        hp_ -= 5;
    }
}

Vector3 MiniBoss::GetCenterPosition() const
{
    // ボスの中心座標＝現在のワールド座標を返す
    return GetWorldTransform().GetTranslate();
}
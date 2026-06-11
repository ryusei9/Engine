#include "BaseCharacter.h"
#include <Object3DCommon.h>

/// <summary>
/// 初期化処理
/// </summary>
void BaseCharacter::Initialize()
{
	// シングルトンインスタンスからInputを取得し保持
	input_ = Input::GetInstance();

	// Object3dCommon経由でデフォルトカメラを取得し保持
	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();

	// ワールド変換（位置・回転・スケール等）の初期化
	worldTransform_.Initialize();
}

/// <summary>
/// 更新処理
/// </summary>
void BaseCharacter::Update()
{
	// 3Dオブジェクトに対して現在のカメラを設定
	object3d_->SetCamera(camera_);
	// ワールド変換の平行移動成分を3Dオブジェクトに反映
	object3d_->SetTranslate(worldTransform_.GetTranslate());
	// ワールド変換の回転成分を3Dオブジェクトに反映
	object3d_->SetRotate(worldTransform_.GetRotate());
	// ワールド変換のスケール成分を3Dオブジェクトに反映
	object3d_->SetScale(worldTransform_.GetScale());
	// 3Dオブジェクト自身の更新処理を実行
	object3d_->Update();
}

/// <summary>
/// 描画処理
/// </summary>
void BaseCharacter::Draw()
{
	// 3Dオブジェクトの描画を実行
	object3d_->Draw();
}

/// <summary>
/// 衝突判定時のコールバック
/// </summary>
/// <param name="other">衝突した相手のColliderポインタ</param>
void BaseCharacter::OnCollision(Collider* other)
{
	// 基底クラスのためデフォルトでは何もしない
	(void)other;
}

/// <summary>
/// 中心座標の取得
/// </summary>
/// <returns>キャラクターの中心座標</returns>
Vector3 BaseCharacter::GetCenterPosition() const
{
	// 基底クラスのためデフォルトでは原点を返す
	return Vector3();
}

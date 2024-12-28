#include "EnemyBullet.h"
#include <Add.h>

EnemyBullet::~EnemyBullet() {

}

void EnemyBullet::Initialize(Object3d* model, const Vector3& position, const Vector3& velocity) {
	// NULLポインタチェック
	assert(model);
	model_ = model;
	// テクスチャ読み込み
	//textureHandle_ = TextureManager::Load("enemy_bullet.png");
	// ワールドトランスフォームの初期化
	transform_.translate = model_->GetTranslate();
	transform_.scale = model_->GetScale();
	transform_.rotate = model_->GetRotate();
	// 引数で受け取った初期座標をセット
	transform_.translate = position;
	// 引数で受け取った速度をメンバ変数に代入
	velocity_ = velocity;
	
}

void EnemyBullet::Update() {
	// 座標を移動させる(1フレーム分の移動量を差し込む)
	transform_.translate = Add(transform_.translate, velocity_);
	model_->SetTranslate(transform_.translate);
	model_->SetScale(transform_.scale);
	model_->SetRotate(transform_.rotate);

	// モデルの更新
	model_->Update();
	// 時間経過でデス
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}
}

void EnemyBullet::Draw(){
	model_->Draw();
}

Vector3 EnemyBullet::GetWorldPosition() {
	// ワールド座標を入れる変数
	Vector3 worldPos = {};
	// ワールド行列の平行移動成分を取得(ワールド座標)
	worldPos.x = transform_.translate.x;
	worldPos.y = transform_.translate.y;
	worldPos.z = transform_.translate.z;

	return worldPos;
}

void EnemyBullet::OnCollision() {
	isDead_ = true;

}
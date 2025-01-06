#include "Enemy.h"
#include "cassert"
#include <DierctXGame/application/object/Player.h>
#include <DierctXGame/application/scene/GameScene.h>
#include <Add.h>
#include <Subtract.h>
#include <Normalize.h>
#include <EnemyBullet.h>
#include <Lerp.h>
#include <thread>
Enemy::~Enemy()
{
	for (EnemyBullet* bullet : bullets_) {
		delete bullet;
	}
}

void Enemy::Initialize(Object3d* model,Object3d* bulletModel, const Vector3& position)
{
	// NULLポインタチェック
	assert(model);
	model_ = model;
	bulletModel_ = bulletModel;
	// テクスチャ読み込み
	//textureHandle_ = TextureManager::Load("enemy.png");
	SetInitialize(position);
}

void Enemy::Update()
{
	bullets_.remove_if([](EnemyBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
		});
	for(EnemyBullet* bullet : bullets_) {
		bullet->Update();
	}
	if (hp_ <= 0) {
		isDead_ = true;
	}
	// 敵の行動パターン
	switch (phese_) {
		// 接近フェーズ
	case Phese::Approach:
	default:
		ApproachPheseUpdate();
		break;
		// 離脱フェーズ
	case Phese::Battle:
		BattlePheseUpdate();
		break;
	}
	//ImGui::End();
	

	model_->SetTranslate(transform_.translate);
	model_->SetScale(transform_.scale);
	model_->SetRotate(transform_.rotate);

	// モデルの更新
	model_->Update();
}

void Enemy::Draw()
{
	model_->Draw();
	for (EnemyBullet* bullet : bullets_) {
		bullet->Draw();
	}
}

void Enemy::ApproachPheseUpdate()
{
	// 速度
	const float kEnemyApproachSpeed = -0.01f;
	Vector3 velocity_ = { kEnemyApproachSpeed, 0, 0 };
	// 移動(ベクトルを加算)
	transform_.translate = Add(transform_.translate, velocity_);
	// 既定の位置に到達したら離脱
	if (transform_.translate.x < 2.0f) {
		
		fireTimer = 300;
		phese_ = Phese::Battle;
	}
	
}

void Enemy::BattlePheseUpdate()
{
	transform_.rotate.y = Lerp(transform_.rotate.y, -0.8f, 0.01f);

	//ImGui::Text("Phese : Leave");
	// 速度
	//const float kEnemyLeaveSpeed = 0.1f;
	//Vector3 velocity_ = { kEnemyLeaveSpeed, 0, 0 };
	//// 移動(ベクトルを加算)
	//transform_.translate = Add(transform_.translate, velocity_);
	// 発射タイマーカウントダウン
	fireTimer--;
	// 指定時間に達した
	if (fireTimer <= 0) {
		// 弾を発射
		Fire();
		// 発射タイマーを初期化
		fireTimer = kFireInterval;
	}
}

void Enemy::Fire()
{
	assert(player_);
	// 弾の速度
	const float kBulletSpeed = 0.2f;

	// 自キャラのワールド座標を取得する
	Vector3 playerPos = player_->GetWorldPosition();

	// 敵キャラのワールド座標を取得する
	Vector3 enemyPos = GetWorldPosition();

	// 敵キャラから自キャラへの差分ベクトルを求める
	Vector3 diff = Subtract(playerPos, enemyPos);

	// ベクトルの正規化
	diff = Normalize(diff);

	// ベクトルの長さを、速さに合わせる
	diff = Multiply(kBulletSpeed, diff);

	// 弾を生成し、初期化
	EnemyBullet* newBullet = new EnemyBullet();
	newBullet->Initialize(bulletModel_, transform_.translate, diff);

	// 弾を登録する
	bullets_.push_back(newBullet);
	gameScene_->AddEnemyBullet(newBullet);
}

void Enemy::ApproachPheseInitialize()
{
	// 発射タイマーを初期化
	fireTimer = kFireInterval;
}

Vector3 Enemy::GetWorldPosition()
{
	return transform_.translate;
}

void Enemy::OnCollision()
{
	hp_ -= 1;
}

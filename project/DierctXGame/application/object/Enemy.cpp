#include "Enemy.h"
#include "cassert"
#include <DierctXGame/application/object/Player.h>
#include <DierctXGame/application/scene/GameScene.h>
#include <Add.h>
#include <Subtract.h>
#include <Normalize.h>
#include <DierctXGame/application/object/EnemyBullet.h>
#include <Lerp.h>
#include <thread>
#include <Lerp.h>
#include <imGui.h>
Enemy::~Enemy()
{
	for (auto& bullet : bullets_) {
		delete bullet;
	}
	bullets_.clear();
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
	for(auto& bullet : bullets_) {
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
		// 攻撃フェーズ
	case Phese::Attack:
		AttackPheseUpdate();
		break;
	}
	//ImGui::End();
	

	model_->SetTranslate(worldTransform_.translate_);
	model_->SetScale(worldTransform_.scale_);
	model_->SetRotate(worldTransform_.rotate_);

	// モデルの更新
	model_->Update();
}

void Enemy::Draw()
{
	model_->Draw();
	for (auto& bullet : bullets_) {
		bullet->Draw();
	}
}

void Enemy::ApproachPheseUpdate()
{
	// 速度
	const float kEnemyApproachSpeed = -0.01f;
	Vector3 velocity_ = { kEnemyApproachSpeed, 0, 0 };
	// 移動(ベクトルを加算)
	worldTransform_.translate_ = Add(worldTransform_.translate_, velocity_);
	// 既定の位置に到達したら離脱
	if (worldTransform_.translate_.x < 2.0f) {
		
		fireTimer = 300;
		phese_ = Phese::Battle;
		battleStartTime_ = std::chrono::steady_clock::now(); // 戦闘フェーズの開始時間を記録
	
	}
	
}

void Enemy::BattlePheseUpdate()
{
	worldTransform_.translate_.x = Lerp(worldTransform_.translate_.x, 2.0f, 0.01f);
	worldTransform_.translate_.y = Lerp(worldTransform_.translate_.y, 0.0f, 0.01f);
	worldTransform_.translate_.z = Lerp(worldTransform_.translate_.z, 0.0f, 0.01f);
	worldTransform_.rotate_.y = Lerp(worldTransform_.rotate_.y, -0.8f, 0.01f);
	worldTransform_.rotate_.z = Lerp(worldTransform_.rotate_.z, 0.0f, 0.01f);
	//ImGui::Text("Phese : Leave");
	// 速度
	//const float kEnemyLeaveSpeed = 0.1f;
	//Vector3 velocity_ = { kEnemyLeaveSpeed, 0, 0 };
	//// 移動(ベクトルを加算)
	//worldTransform_.translate = Add(worldTransform_.translate, velocity_);
	// 発射タイマーカウントダウン
	fireTimer--;
	// 指定時間に達した
	if (fireTimer <= 0) {
		// 弾を発射
		Fire();
		// 発射タイマーを初期化
		fireTimer = kFireInterval;
	}
	// 戦闘フェーズが終了したら攻撃フェーズに移行
	if (std::chrono::steady_clock::now() >= battleStartTime_ + battleDuration_) {
		phese_ = Phese::Attack;
		attackEndTime_ = std::chrono::steady_clock::now() + attackDuration_;
	}
}

void Enemy::AttackPheseUpdate()
{
	worldTransform_.translate_.x = Lerp(worldTransform_.translate_.x, 0.0f, 0.01f);
	worldTransform_.translate_.y = Lerp(worldTransform_.translate_.y, 0.0f, 0.01f);
	worldTransform_.translate_.z = Lerp(worldTransform_.translate_.z, 10.0f, 0.01f);
	worldTransform_.rotate_.y = Lerp(worldTransform_.rotate_.y, -1.5f, 0.01f);
	worldTransform_.rotate_.z = Lerp(worldTransform_.rotate_.z, 6.3f, 0.01f);
	// 発射タイマーカウントダウン
	fireTimer--;
	// 指定時間に達した
	if (fireTimer <= 0) {
		// 弾を発射
		Fire();
		switch (phese_) {
		case Phese::Battle:
			// 発射タイマーを初期化
			fireTimer = kFireInterval;
			break;
		case Phese::Attack:
			// 発射タイマーを初期化
			fireTimer = kFireInterval2;
			break;
		}
	}

	// 攻撃フェーズが終了したかどうかを確認
	if (std::chrono::steady_clock::now() >= attackEndTime_) {
		phese_ = Phese::Battle; // 次のフェーズに移行
		battleStartTime_ = std::chrono::steady_clock::now(); // 戦闘フェーズの開始時間を再度記録
	}
	
}

void Enemy::Fire()
{
	assert(player_);
	// 弾の速度
	
	switch (phese_) {
	case Phese::Battle:
		kBulletSpeed = 0.1f;
		break;
	case Phese::Attack:
		kBulletSpeed = 0.5f;
		break;
	}

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
	newBullet->Initialize(bulletModel_, worldTransform_.translate_, diff);

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
	// ワールド座標を入れる変数
	Vector3 worldPos = {};
	// ワールド行列の平行移動成分を取得(ワールド座標)
	worldPos.x = worldTransform_.translate_.x;
	worldPos.y = worldTransform_.translate_.y;
	worldPos.z = worldTransform_.translate_.z;

	return worldPos;
}

void Enemy::OnCollision()
{
	hp_ -= 1;
}

void Enemy::ImGuiDraw()
{
	ImGui::Begin("Enemy");
	ImGui::Text("HP : %d", hp_);
	ImGui::Text("Position : (%.2f,%.2f,%.2f)", worldTransform_.translate_.x, worldTransform_.translate_.y, worldTransform_.translate_.z);
	ImGui::Text("Phese : %d", static_cast<int>(phese_));
	ImGui::Text("FireTimer : %d", fireTimer);
	ImGui::Text("Dead : %s", isDead_ ? "true" : "false");
	ImGui::Text("BulletCount : %d", bullets_.size());
	ImGui::Text("AttackEndTime : %d", attackEndTime_);
	ImGui::Text("BattleStartTime : %d", battleStartTime_);
	ImGui::Text("BattleDuration : %d", battleDuration_);
	ImGui::Text("AttackDuration : %d", attackDuration_);
	ImGui::Text("BulletSpeed : %f", kBulletSpeed);
	ImGui::Text("FireInterval : %d", kFireInterval);
	ImGui::Text("FireInterval2 : %d", kFireInterval2);
	ImGui::Text("Player : %p", player_);
	ImGui::Text("GameScene : %p", gameScene_);
	ImGui::Text("Model : %p", model_);
	ImGui::Text("BulletModel : %p", bulletModel_);
	ImGui::Text("TextureHandle : %d", textureHandle_);
	ImGui::Text("Transform : ");
	ImGui::Text("Translate : (%.2f,%.2f,%.2f)", worldTransform_.translate_.x, worldTransform_.translate_.y, worldTransform_.translate_.z);
	ImGui::Text("Scale : (%.2f,%.2f,%.2f)", worldTransform_.scale_.x, worldTransform_.scale_.y, worldTransform_.scale_.z);
	for (auto& bullet : bullets_) {
		bullet->ImGuiDraw();
	}
	ImGui::End();
}

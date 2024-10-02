#include "Enemy.h"
#include <TextureManager.h>
#include "cassert"
#include "ImGuiManager.h"
#include "Player.h"
#include "GameScene.h"

Enemy::~Enemy() {
	delete mathMatrix_;
	/*for (EnemyBullet* bullet : bullets_) {
		delete bullet;
	}*/
}

void Enemy::Initialize(Model* model, const Vector3& position) {
	// NULLポインタチェック
	assert(model);
	model_ = model;
	// テクスチャ読み込み
	textureHandle_ = TextureManager::Load("enemy.png");
	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	// 引数で受け取った初期座標をセット
	worldTransform_.translation_ = position;

	mathMatrix_ = new MathMatrix();
	ApproachPheseInitialize();
}

void Enemy::Update() {
	///ImGui::Begin("enemy");
	///ImGui::DragFloat3("translation", &worldTransform_.translation_.x, 0.1f);

	// 敵の行動パターン
	switch (phese_) {
		// 接近フェーズ
	case Phese::Approach:
	default:
		ApproachPheseUpdate();
		break;
		// 離脱フェーズ
	case Phese::Leave:
		LeavePheseUpdate();
		break;
	}
	//ImGui::End();

	worldTransform_.UpdateMatrix(); 
}

void Enemy::Draw(const ViewProjection& viewProjection) {
	// 3Dモデルを描画
	model_->Draw(worldTransform_, viewProjection, textureHandle_);
}

void Enemy::ApproachPheseUpdate() {
	//ImGui::Text("Phese : Approach");
	// 速度
	const float kEnemyApproachSpeed = -0.1f;
	Vector3 velocity_ = {0, 0, kEnemyApproachSpeed};
	// 移動(ベクトルを加算)
	worldTransform_.translation_ = mathMatrix_->Add(worldTransform_.translation_, velocity_);
	// 既定の位置に到達したら離脱
	if (worldTransform_.translation_.z < 0.0f) {
		phese_ = Phese::Leave;
	}
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

void Enemy::LeavePheseUpdate() {
	//ImGui::Text("Phese : Leave");
	// 速度
	const float kEnemyLeaveSpeed = 0.1f;
	Vector3 velocity_ = {0, 0, kEnemyLeaveSpeed};
	// 移動(ベクトルを加算)
	worldTransform_.translation_ = mathMatrix_->Add(worldTransform_.translation_, velocity_);
}

void Enemy::Fire() {
	assert(player_);
	// 弾の速度
	const float kBulletSpeed = 1.2f;
	
	// 自キャラのワールド座標を取得する
	Vector3 playerPos = player_->GetWorldPosition();

	// 敵キャラのワールド座標を取得する
	Vector3 enemyPos = GetWorldPosition();

	// 敵キャラから自キャラへの差分ベクトルを求める
	Vector3 diff = mathMatrix_->Subtract(playerPos,enemyPos);

	// ベクトルの正規化
	diff = mathMatrix_->Normalize(diff);

	// ベクトルの長さを、速さに合わせる
	diff = mathMatrix_->Multiply(kBulletSpeed, diff);
	
	// 弾を生成し、初期化
	EnemyBullet* newBullet = new EnemyBullet();
	newBullet->Initialize(model_, worldTransform_.translation_, diff);

	// 弾を登録する
	//bullets_.push_back(newBullet);
	gameScene_->AddEnemyBullet(newBullet);
}

void Enemy::ApproachPheseInitialize() {
	// 発射タイマーを初期化
	fireTimer = kFireInterval;
}

Vector3 Enemy::GetWorldPosition() {
	// ワールド座標を入れる変数
	Vector3 worldPos = {};
	// ワールド行列の平行移動成分を取得(ワールド座標)
	worldPos.x = worldTransform_.translation_.x;
	worldPos.y = worldTransform_.translation_.y;
	worldPos.z = worldTransform_.translation_.z;

	return worldPos;
}

void Enemy::OnCollision() { isDead_ = true; }


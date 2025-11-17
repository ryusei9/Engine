#include "EnemyAttack.h"
#include <cmath>
#include <algorithm>
#include <imgui.h>
#include <Enemy.h>
#include <Player.h>

// 円周率
constexpr float PI = 3.1415926535f;

// 追加: 弾速スケール（0.5で半分）
constexpr float BULLET_SPEED_SCALE = 0.5f;
// パターン1: 画面右側で上下移動しつつ2秒ごとに扇形弾
void EnemyAttackPatternFan::Update(Enemy* enemy, Player*, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) {
	// 上下移動
	float& y = enemy->GetWorldTransform().translate_.y;
	y += moveDir_ * 0.02f;
	if (y > 2.0f) moveDir_ = -1.0f;
	if (y < -2.0f) moveDir_ = 1.0f;

	// 2秒ごとに5発扇形弾
	shotTimer_ += deltaTime;
	if (shotTimer_ >= 2.0f) {
		float baseAngle = PI; // 左向き
		float spread = PI / 3.0f; // 扇の広がり
		for (int i = 0; i < 5; ++i) {
			float angle = baseAngle - spread / 2 + spread * (float(i) / 4.0f);
			Vector3 v = { std::cos(angle) * 0.15f, std::sin(angle) * 0.15f, 0.0f };
			auto bullet = std::make_unique<EnemyBullet>();
			bullet->Initialize(enemy->GetWorldTransform().translate_, v);
			bullet->Update();
			bullets.push_back(std::move(bullet));
		}
		shotTimer_ = 0.0f;
	}
}
// ImGui描画
void EnemyAttackPatternFan::DrawImGui(int idx, bool selected) {
#ifdef USE_IMGUI
	if (ImGui::Selectable(GetName(), selected)) {}
#endif
}

// パターン2: 右側中央で自機狙い弾連射
void EnemyAttackPatternAimed::Update(Enemy* enemy, Player* player, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) {
	// 位置固定
	//enemy->GetWorldTransform().translate_ = { 3.0f, 0.0f, 0.0f };
	// 0.2秒ごとに自機狙い弾
	shotTimer_ += deltaTime;
	if (shotTimer_ >= 2.0f && player) {
		Vector3 toPlayer = player->GetCenterPosition() - enemy->GetWorldTransform().translate_;
		float len = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z);
		if (len > 0.01f) {
			float speed = 0.18f * BULLET_SPEED_SCALE;
			Vector3 v = { toPlayer.x / len * speed,
						  toPlayer.y / len * speed,
						  0.0f };
			auto bullet = std::make_unique<EnemyBullet>();
			bullet->Initialize(enemy->GetWorldTransform().translate_, v);
			bullet->Update();
			bullets.push_back(std::move(bullet));
		}
		shotTimer_ = 0.0f;
	}
}

// ImGui描画
void EnemyAttackPatternAimed::DrawImGui(int idx, bool selected) {
#ifdef USE_IMGUI
	if (ImGui::Selectable(GetName(), selected)) {}
#endif
}

// パターン3: 全方位弾+左突進→左で消えて右から復活
void EnemyAttackPatternRush::Update(Enemy* enemy, Player*, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) {
	if (!rushing_) {
		enemy->GetWorldTransform().translate_ = { 5.0f, 0.0f, 0.0f };
		rushing_ = true;
		shotTimer_ = 0.0f;
	}
	enemy->GetWorldTransform().translate_.x -= 0.08f;
	shotTimer_ += deltaTime;
	if (shotTimer_ >= 0.3f) {
		for (int i = 0; i < 12; ++i) {
			float angle = 2 * PI * i / 12;
			Vector3 v = { std::cos(angle) * 0.13f, std::sin(angle) * 0.13f, 0.0f };
			auto bullet = std::make_unique<EnemyBullet>();
			bullet->Initialize(enemy->GetWorldTransform().translate_, v);
			bullet->Update();
			bullets.push_back(std::move(bullet));
		}
		shotTimer_ = 0.0f;
	}
	if (enemy->GetWorldTransform().translate_.x < -4.0f) {
		rushing_ = false;
	}
}

void EnemyAttackPatternWait::Update(Enemy* enemy, Player*, std::list<std::unique_ptr<EnemyBullet>>&,float deltaTime) {
	
}

// ImGui描画
void EnemyAttackPatternWait::DrawImGui(int idx, bool selected) {
#ifdef USE_IMGUI
	if (ImGui::Selectable(GetName(), selected)) {}
#endif
}


// ImGui描画
void EnemyAttackPatternRush::DrawImGui(int idx, bool selected) {
#ifdef USE_IMGUI
	if (ImGui::Selectable(GetName(), selected)) {}
#endif
}

// EnemyAttack本体
EnemyAttack::EnemyAttack() {
	patterns_.push_back(std::make_unique<EnemyAttackPatternFan>());   // 0
	patterns_.push_back(std::make_unique<EnemyAttackPatternAimed>()); // 1
	patterns_.push_back(std::make_unique<EnemyAttackPatternRush>());  // 2
	patterns_.push_back(std::make_unique<EnemyAttackPatternWait>());  // 3
}
void EnemyAttack::Update(Enemy* enemy, Player* player, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) {
	// パターン遷移管理
	patternTimer_ += deltaTime;

	// パターン1→待機
	if (currentPattern_ == 0 && patternTimer_ >= 10.0f) {
		SetPattern(3); // 3: 待機
		patternTimer_ = 0.0f;
		return;
	}
	// パターン2→待機
	/*if (currentPattern_ == 1 && patternTimer_ >= 10.0f) {
		SetPattern(3);
		patternTimer_ = 0.0f;
		return;
	}*/
	// パターン3→待機（突進しきったら）
	if (currentPattern_ == 2) {
		if (!pattern3Rushed_ && enemy->GetWorldTransform().translate_.x < -4.0f) {
			pattern3Rushed_ = true;
			enemy->GetWorldTransform().translate_ = { 4.0f, 0.0f, 0.0f };
			SetPattern(3);
			patternTimer_ = 0.0f;
			return;
		}
		if (enemy->GetWorldTransform().translate_.x >= 3.0f) {
			pattern3Rushed_ = false;
		}
	}

	// パターン実行
	if (currentPattern_ >= 0 && currentPattern_ < int(patterns_.size())) {
		patterns_[currentPattern_]->Update(enemy, player, bullets, deltaTime);
	}
}

// ImGui描画
void EnemyAttack::DrawImGui() {
#ifdef USE_IMGUI
	for (int i = 0; i < int(patterns_.size()); ++i) {
		bool selected = (i == currentPattern_);
		if (ImGui::Selectable(patterns_[i]->GetName(), selected)) {
			currentPattern_ = i;
		}
	}
#endif
}

// パターン設定
void EnemyAttack::SetPattern(int idx) {
	if (idx >= 0 && idx < int(patterns_.size())) currentPattern_ = idx;
}
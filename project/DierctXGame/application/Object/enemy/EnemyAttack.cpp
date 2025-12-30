#include "EnemyAttack.h"
#include <cmath>
#include <algorithm>
#include <imgui.h>
#include <Enemy.h>
#include <Player.h>

// パターン1: 画面右側で上下移動しつつ扇形弾
void EnemyAttackPatternFan::Update(Enemy* enemy, Player*, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) {
	// 上下移動
	Vector3 t = enemy->GetWorldTransform().GetTranslate();
	t.y += moveDir_ * EnemyAttackDefaults::kFanMoveSpeedY;
	if (t.y > EnemyAttackDefaults::kFanMoveRangeY) moveDir_ = -1.0f;
	if (t.y < -EnemyAttackDefaults::kFanMoveRangeY) moveDir_ = 1.0f;
	enemy->GetWorldTransform().SetTranslate(t);

	// 扇形弾
	shotTimer_ += deltaTime;
	if (shotTimer_ >= EnemyAttackDefaults::kFanShotIntervalSec) {
		const float baseAngle = EnemyAttackDefaults::kFanBaseAngle;
		const float spread = EnemyAttackDefaults::kFanSpread;
		for (int32_t i = 0; i < EnemyAttackDefaults::kFanShotCount; ++i) {
			float angle = baseAngle - spread / 2.0f + spread * (float(i) / float(EnemyAttackDefaults::kFanShotCount - 1));
			Vector3 v = { std::cos(angle) * EnemyAttackDefaults::kFanBulletSpeed,
						  std::sin(angle) * EnemyAttackDefaults::kFanBulletSpeed,
						  0.0f };
			auto bullet = std::make_unique<EnemyBullet>();
			bullet->Initialize(enemy->GetWorldTransform().GetTranslate(), v);
			bullet->Update();
			bullets.push_back(std::move(bullet));
		}
		shotTimer_ = 0.0f;
	}
}

// ImGui描画
void EnemyAttackPatternFan::DrawImGui(int32_t idx, bool selected) {
#ifdef USE_IMGUI
	if (ImGui::Selectable(GetName(), selected)) {}
#endif
}

// パターン2: 右側中央で自機狙い弾連射
void EnemyAttackPatternAimed::Update(Enemy* enemy, Player* player, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) {
	shotTimer_ += deltaTime;
	if (shotTimer_ >= EnemyAttackDefaults::kAimedShotIntervalSec && player) {
		Vector3 toPlayer = player->GetCenterPosition() - enemy->GetWorldTransform().GetTranslate();
		float len = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z);
		if (len > EnemyAttackDefaults::kAimedMinLen) {
			float speed = EnemyAttackDefaults::kAimedBulletSpeed * EnemyAttackDefaults::kBulletSpeedScale;
			Vector3 v = { toPlayer.x / len * speed,
						  toPlayer.y / len * speed,
						  0.0f };
			auto bullet = std::make_unique<EnemyBullet>();
			bullet->Initialize(enemy->GetWorldTransform().GetTranslate(), v);
			bullet->Update();
			bullets.push_back(std::move(bullet));
		}
		shotTimer_ = 0.0f;
	}
}

// ImGui描画
void EnemyAttackPatternAimed::DrawImGui(int32_t idx, bool selected) {
#ifdef USE_IMGUI
	if (ImGui::Selectable(GetName(), selected)) {}
#endif
}

// パターン3: 全方位弾+左突進→左で消えて右から復活
void EnemyAttackPatternRush::Update(Enemy* enemy, Player*, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) {
	if (!rushing_) {
		Vector3 t = enemy->GetWorldTransform().GetTranslate();
		t.x = EnemyAttackDefaults::kRushStartX;
		enemy->GetWorldTransform().SetTranslate(t);
		rushing_ = true;
		shotTimer_ = 0.0f;
	}

	// 左へ移動
	Vector3 t = enemy->GetWorldTransform().GetTranslate();
	t.x -= EnemyAttackDefaults::kRushSpeedX;
	enemy->GetWorldTransform().SetTranslate(t);

	// 全方位弾
	shotTimer_ += deltaTime;
	if (shotTimer_ >= EnemyAttackDefaults::kRushShotIntervalSec) {
		for (int32_t i = 0; i < EnemyAttackDefaults::kRushRingCount; ++i) {
			float angle = 2.0f * EnemyAttackDefaults::kPi * float(i) / float(EnemyAttackDefaults::kRushRingCount);
			Vector3 v = { std::cos(angle) * EnemyAttackDefaults::kRushRingSpeed,
						  std::sin(angle) * EnemyAttackDefaults::kRushRingSpeed,
						  0.0f };
			auto bullet = std::make_unique<EnemyBullet>();
			bullet->Initialize(enemy->GetWorldTransform().GetTranslate(), v);
			bullet->Update();
			bullets.push_back(std::move(bullet));
		}
		shotTimer_ = 0.0f;
	}

	// 左端で終了
	if (enemy->GetWorldTransform().GetTranslate().x < EnemyAttackDefaults::kRushLeftEndX) {
		rushing_ = false;
	}
}

void EnemyAttackPatternWait::Update(Enemy* enemy, Player*, std::list<std::unique_ptr<EnemyBullet>>&, float /*deltaTime*/) {
	// 必要なら待機のイージング移動などをここで実装
}

// ImGui描画
void EnemyAttackPatternWait::DrawImGui(int32_t idx, bool selected) {
#ifdef USE_IMGUI
	if (ImGui::Selectable(GetName(), selected)) {}
#endif
}

// ImGui描画
void EnemyAttackPatternRush::DrawImGui(int32_t idx, bool selected) {
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
	if (currentPattern_ == 0 && patternTimer_ >= EnemyAttackDefaults::kPattern1DurationSec) {
		SetPattern(3); // 3: 待機
		patternTimer_ = 0.0f;
		return;
	}
	
	// パターン3→待機（突進しきったら）
	if (currentPattern_ == 2) {
		if (!pattern3Rushed_ && enemy->GetWorldTransform().GetTranslate().x < EnemyAttackDefaults::kRushLeftEndX) {
			pattern3Rushed_ = true;
			Vector3 t = enemy->GetWorldTransform().GetTranslate();
			t.x = EnemyAttackDefaults::kRushRespawnX;
			t.y = 0.0f;
			t.z = t.z;
			enemy->GetWorldTransform().SetTranslate(t);
			SetPattern(3);
			patternTimer_ = 0.0f;
			return;
		}
		if (enemy->GetWorldTransform().GetTranslate().x >= EnemyAttackDefaults::kRushResetX) {
			pattern3Rushed_ = false;
		}
	}

	// パターン実行
	if (currentPattern_ >= 0 && currentPattern_ < int32_t(patterns_.size())) {
		patterns_[currentPattern_]->Update(enemy, player, bullets, deltaTime);
	}
}

// ImGui描画
void EnemyAttack::DrawImGui() {
#ifdef USE_IMGUI
	for (int32_t i = 0; i < int32_t(patterns_.size()); ++i) {
		bool selected = (i == currentPattern_);
		if (ImGui::Selectable(patterns_[i]->GetName(), selected)) {
			currentPattern_ = i;
		}
	}
#endif
}

// パターン設定
void EnemyAttack::SetPattern(int32_t idx) {
	if (idx >= 0 && idx < int32_t(patterns_.size())) currentPattern_ = idx;
}
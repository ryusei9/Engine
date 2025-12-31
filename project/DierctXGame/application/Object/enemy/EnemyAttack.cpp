#include "EnemyAttack.h"
#include <cmath>
#include <algorithm>
#include <Enemy.h>
#include <Player.h>
#include "JsonLoader.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif

// パターン1: 画面右側で上下移動しつつ扇形弾
void EnemyAttackPatternFan::Update(Enemy* enemy, Player*, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) {
	// 上下移動
	Vector3 t = enemy->GetWorldTransform().GetTranslate();
	t.y += moveDir_ * parameters_.fanMoveSpeedY;
	if (t.y > parameters_.fanMoveRangeY) {
		moveDir_ = -1.0f;
	}
	if (t.y < -parameters_.fanMoveRangeY) {
		moveDir_ = 1.0f;
	}
	enemy->GetWorldTransform().SetTranslate(t);

	// 扇形弾
	shotTimer_ += deltaTime;
	if (shotTimer_ >= parameters_.fanShotIntervalSec) {
		const float baseAngle = parameters_.fanBaseAngle;
		const float spread = parameters_.fanSpread;
		for (int32_t i = 0; i < parameters_.fanShotCount; ++i) {
			float angle = baseAngle - spread / 2.0f + spread * (float(i) / float(parameters_.fanShotCount - 1));
			Vector3 v = { std::cos(angle) * parameters_.fanBulletSpeed,
						  std::sin(angle) * parameters_.fanBulletSpeed,
						  0.0f };
			auto bullet = std::make_unique<EnemyBullet>();
			bullet->Initialize(enemy->GetWorldTransform().GetTranslate(), v, "enemyBulletParameters");
			bullet->Update();
			bullets.push_back(std::move(bullet));
		}
		shotTimer_ = 0.0f;
	}
}

void EnemyAttackPatternFan::DrawImGui(int32_t idx, bool selected) {
#ifdef USE_IMGUI
	if (ImGui::Selectable(GetName(), selected)) {}
#endif
}

// パターン2: 右側中央で自機狙い弾連射
void EnemyAttackPatternAimed::Update(Enemy* enemy, Player* player, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) {
	shotTimer_ += deltaTime;
	if (shotTimer_ >= parameters_.aimedShotIntervalSec && player) {
		Vector3 toPlayer = player->GetCenterPosition() - enemy->GetWorldTransform().GetTranslate();
		float len = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z);
		if (len > parameters_.aimedMinLen) {
			float speed = parameters_.aimedBulletSpeed * parameters_.bulletSpeedScale;
			Vector3 v = { toPlayer.x / len * speed,
						  toPlayer.y / len * speed,
						  0.0f };
			auto bullet = std::make_unique<EnemyBullet>();
			bullet->Initialize(enemy->GetWorldTransform().GetTranslate(), v, "enemyBulletParameters");
			bullet->Update();
			bullets.push_back(std::move(bullet));
		}
		shotTimer_ = 0.0f;
	}
}

void EnemyAttackPatternAimed::DrawImGui(int32_t idx, bool selected) {
#ifdef USE_IMGUI
	if (ImGui::Selectable(GetName(), selected)) {}
#endif
}

// パターン3: 全方位弾+左突進→左で消えて右から復活
void EnemyAttackPatternRush::Update(Enemy* enemy, Player*, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) {
	if (!rushing_) {
		Vector3 t = enemy->GetWorldTransform().GetTranslate();
		t.x = parameters_.rushStartX;
		enemy->GetWorldTransform().SetTranslate(t);
		rushing_ = true;
		shotTimer_ = 0.0f;
	}

	// 左へ移動
	Vector3 t = enemy->GetWorldTransform().GetTranslate();
	t.x -= parameters_.rushSpeedX;
	enemy->GetWorldTransform().SetTranslate(t);

	// 全方位弾
	shotTimer_ += deltaTime;
	if (shotTimer_ >= parameters_.rushShotIntervalSec) {
		for (int32_t i = 0; i < parameters_.rushRingCount; ++i) {
			float angle = 2.0f * EnemyAttackDefaults::kPi * float(i) / float(parameters_.rushRingCount);
			Vector3 v = { std::cos(angle) * parameters_.rushRingSpeed,
						  std::sin(angle) * parameters_.rushRingSpeed,
						  0.0f };
			auto bullet = std::make_unique<EnemyBullet>();
			bullet->Initialize(enemy->GetWorldTransform().GetTranslate(), v, "enemyBulletParameters");
			bullet->Update();
			bullets.push_back(std::move(bullet));
		}
		shotTimer_ = 0.0f;
	}

	// 左端で終了
	if (enemy->GetWorldTransform().GetTranslate().x < parameters_.rushLeftEndX) {
		rushing_ = false;
	}
}

void EnemyAttackPatternRush::DrawImGui(int32_t idx, bool selected) {
#ifdef USE_IMGUI
	if (ImGui::Selectable(GetName(), selected)) {}
#endif
}

// パターン4: 待機
void EnemyAttackPatternWait::Update(Enemy* enemy, Player*, std::list<std::unique_ptr<EnemyBullet>>&, float /*deltaTime*/) {
	// 待機中は特に何もしない
}

void EnemyAttackPatternWait::DrawImGui(int32_t idx, bool selected) {
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

void EnemyAttack::Initialize(const std::string& parameterFileName) {
	// パラメータファイルから読み込み
	parameters_ = JsonLoader::LoadEnemyAttackParameters(parameterFileName);

	// 各パターンにパラメータを設定
	for (auto& pattern : patterns_) {
		pattern->SetParameters(parameters_);
	}
}

void EnemyAttack::Update(Enemy* enemy, Player* player, std::list<std::unique_ptr<EnemyBullet>>& bullets, float deltaTime) {
	// パターン遷移管理
	patternTimer_ += deltaTime;

	// パターン1→待機
	if (currentPattern_ == 0 && patternTimer_ >= parameters_.pattern1DurationSec) {
		SetPattern(3); // 3: 待機
		patternTimer_ = 0.0f;
		return;
	}
	
	// パターン3→待機（突進しきったら）
	if (currentPattern_ == 2) {
		if (!pattern3Rushed_ && enemy->GetWorldTransform().GetTranslate().x < parameters_.rushLeftEndX) {
			pattern3Rushed_ = true;
			Vector3 t = enemy->GetWorldTransform().GetTranslate();
			t.x = parameters_.rushRespawnX;
			t.y = 0.0f;
			enemy->GetWorldTransform().SetTranslate(t);
			SetPattern(3);
			patternTimer_ = 0.0f;
			return;
		}
		if (enemy->GetWorldTransform().GetTranslate().x >= parameters_.rushResetX) {
			pattern3Rushed_ = false;
		}
	}

	// パターン実行
	if (currentPattern_ >= 0 && currentPattern_ < int32_t(patterns_.size())) {
		patterns_[currentPattern_]->Update(enemy, player, bullets, deltaTime);
	}
}

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

void EnemyAttack::SetPattern(int32_t idx) {
	if (idx >= 0 && idx < int32_t(patterns_.size())) {
		currentPattern_ = idx;
	}
}

void EnemyAttack::SetParameters(const EnemyAttackParameters& params) {
	parameters_ = params;
	// 各パターンにパラメータを設定
	for (auto& pattern : patterns_) {
	 pattern->SetParameters(parameters_);
	}
}
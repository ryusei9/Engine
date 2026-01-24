#pragma once
#include "BaseCharacter.h"

/// <summary>
/// ボスキャラクタークラス
/// </summary>
class Boss : public BaseCharacter
{
public:
	// 初期化
	void Initialize() override;

	// 更新
	void Update() override;

	// 描画
	void Draw() override;

	// ImGui描画
	void DrawImGui();

	// キャラクターの移動
	void Move() override;

	// キャラクターの攻撃
	void Attack() override;

	// 衝突判定
	void OnCollision(Collider* other) override;
};


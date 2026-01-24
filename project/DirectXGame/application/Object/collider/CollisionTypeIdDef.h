#pragma once
#include <cstdint>

/// ---------- 識別IDの定義 ---------- ///
enum class CollisionTypeIdDef : uint32_t
{
	kDefault,		 // デフォルトID
	kPlayer,		 // プレイヤーID
	kPlayerBullet,	 // プレイヤーの弾ID
	kPlayerChargeBullet, // プレイヤーのチャージ弾ID
	kEnemy,			 // エネミーID
	kEnemyBullet,	 // エネミー弾ID
};
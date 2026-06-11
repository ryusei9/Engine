#include "PlayerChargeBullet.h"
#include <CollisionTypeIdDef.h>
#include "JsonLoader.h"
#include <Player.h>
#include <WorldTransform.h>
#include <algorithm>
#ifdef USE_IMGUI
#include <imgui.h>
#endif
// 静的メンバ変数の初期化
uint32_t PlayerChargeBullet::sNextSerialNumber_ = PlayerChargeBulletDefaults::kSerialStart;

PlayerChargeBullet::PlayerChargeBullet()
{
	// シリアルナンバーを設定
	serialNumber_ = sNextSerialNumber_++;
}

void PlayerChargeBullet::Initialize(Player* player)
{
	// デフォルトパラメータで初期化
	Initialize(player, "");
}

void PlayerChargeBullet::Initialize(Player* player, const std::string& parameterFileName)
{
	// プレイヤーが設定されていない場合は何もしない
	if (!player) {
		return;
	}
	// パラメータファイルから読み込み（空文字列の場合はデフォルトパラメータを使用）
	if (!parameterFileName.empty()) {
		chargeBulletParameters_ = JsonLoader::LoadPlayerChargeBulletParameters(parameterFileName);
	} else {
		chargeBulletParameters_ = defaultChargeBulletParameters_;
	}
	// プレイヤー情報を保持
	player_ = player;


	// ダメージパラメータを適用
	damage_ = chargeBulletParameters_.damage;

	// チャージ弾のコライダーID（基底クラスの初期化後に上書き）
	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kPlayerChargeBullet));

	// デバッグ用レーザーパーティクルの初期化
	debugParticleEmitter_ = std::make_unique<ParticleEmitter>(ParticleManager::GetInstance(), "laser");
	debugParticleEmitter_->SetParticleCount(1);
	debugParticleEmitter_->SetParticleType(ParticleType::Laser);

	// 発射時の相対オフセット初期化
	offset_ = {1.0f, 0.0f, 0.0f };

	// 当たり判定の半径も拡大（パラメータから取得）
	SetRadius(chargeBulletParameters_.radius);
	debugParticleEmitter_->SetRadius(chargeBulletParameters_.radius);
}

void PlayerChargeBullet::Update()
{
	// 必要ならチャージ弾専用の挙動をここに追加
	if (!player_) {
		return;
	}

	Vector3 playerPos = player_->GetPosition();

	// 向きベクトルと長さを元に終点を計算
	Vector3 dir = { 1.0f, 0.0f, 0.0f };
	Vector3 end = playerPos + dir * length_;

	// 線分コライダーの始点と終点を更新
	SetStart(playerPos + offset_);
	SetEnd(end);

	// 経過時間を更新
	timer_ += 1.0f / 60.0f;

	// 指定した持続時間を過ぎたら消滅させる
	if (timer_ >= duration_) {
		isAlive_ = false;
	}
	// デバッグ表示用パーティクル
	DebugLaserParticle(playerPos + offset_, end);
}

void PlayerChargeBullet::Draw()
{
	// チャージ弾専用の描画があれば追加
}

void PlayerChargeBullet::OnCollision(Collider* other)
{
	// チャージ弾は敵を貫通するため、何もしない
	// 寿命で消えるのみ
	(void)other; // 未使用パラメータ警告回避
}

void PlayerChargeBullet::DebugLaserParticle(const Vector3& start, const Vector3& end)
{
	// 始点から終点へのベクトルと長さを算出
	Vector3 dir = end - start;
	float length = Vector3::Length(dir);

	// 長さに応じて分割数を決定
	int segment =
		static_cast<int>(length * 20.0f);

	// 最低保証（分割数が少なすぎないようにする）
	segment = std::max<int>(segment, 10);

	// 線分上にパーティクルを配置して更新
	for (int i = 0; i <= segment; ++i)
	{
		// 配置割合 t を計算
		float t =
			static_cast<float>(i) /
			static_cast<float>(segment);

		// tに基づく座標を計算
		Vector3 pos = start + dir * t;

		// パーティクルの表示座標を設定して更新
		debugParticleEmitter_->SetPosition(pos);
		debugParticleEmitter_->Update();
	}
}

void PlayerChargeBullet::DrawImGui()
{
#ifdef USE_IMGUI
	ImGui::Text("PlayerChargeBullet Parameters:");
	ImGui::SliderFloat("Damage", &chargeBulletParameters_.damage, 0.0f, 100.0f);
	ImGui::SliderFloat("Radius", &chargeBulletParameters_.radius, 0.1f, 5.0f);
	ImGui::SliderFloat("Scale Factor", &chargeBulletParameters_.scaleFactor, 0.1f, 5.0f);
	ImGui::DragFloat3("Offset", &offset_.x, 0.1f);
#endif
}

void PlayerChargeBullet::SetChargeBulletParameters(const PlayerChargeBulletParameters& parameters)
{
	chargeBulletParameters_ = parameters;
	damage_ = chargeBulletParameters_.damage;
	SetRadius(chargeBulletParameters_.radius);
}

void PlayerChargeBullet::SetDefaultChargeBulletParameters(const PlayerChargeBulletParameters& parameters)
{
	defaultChargeBulletParameters_ = parameters;
}

const PlayerChargeBulletParameters& PlayerChargeBullet::GetDefaultChargeBulletParameters()
{
	return defaultChargeBulletParameters_;
}
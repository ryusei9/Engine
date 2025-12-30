#pragma once
#include <string>
#include <Vector3.h>

class ParticleManager;// 前方宣言
/*------パーティクル発生器------*/
class ParticleEmitter
{
public:
	/*------メンバ関数------*/
	// コンストラクタ
	ParticleEmitter(ParticleManager* manager, const std::string& groupName);

	// 更新
	void Update();

	// 発生場所の設定
	void SetPosition(const Vector3& position) { position_ = position; }

	void SetVelocity(const Vector3& velocity) { velocity_ = velocity; } // 追加: パーティクルの速度を設定

	// 一秒あたりの最大発生数の設定
	void SetParticleRate(uint32_t rate) { particleRate_ = rate; }

	void SetParticleCount(uint32_t count) { particleCount_ = count; }
	uint32_t GetParticleCount() const { return particleCount_; }

	// 発生場所の取得
	const Vector3& GetPosition() const { return position_; }

	// 一秒あたりの最大発生数の取得
	uint32_t GetParticleRate() const { return particleRate_; }

	// リングパーティクルを使うか
	void SetUseRingParticle(bool use) { useRingParticle_ = use; }

	// 爆発用かどうか
	void SetExplosion(bool isExplosion) { isExplosion_ = isExplosion; }

	// スラスター用かどうか
	void SetThruster(bool isThruster) { isThruster_ = isThruster; }

	// 煙用かどうか
	void SetSmoke(bool isSmoke) { isSmoke_ = isSmoke; }

	// 爆発用かどうかの取得
	bool IsExplosion() const { return isExplosion_; }

private:
	/*------メンバ変数------*/
	// パーティクルマネージャ
	ParticleManager* manager_ = nullptr;

	// パーティクルグループ名
	std::string groupName_;

	// デルタタイム
	float deltaTime_ = 1.0f / 60.0f;

	// パーティクルの発生位置
	Vector3 position_ = { 0.0f, 0.0f, 0.0f };

	Vector3 velocity_ = { 0.0f, 0.0f, 0.0f }; // 追加: パーティクルの速度

	// パーティクルの一秒あたりの最大発生数
	uint32_t particleRate_ = 10;

	// パーティクルの発生間隔
	float interval_ = 0.0f;

	// リングパーティクルを使うか
	bool useRingParticle_ = false;

	bool isExplosion_ = false; // 追加: 爆発用かどうか

	bool isThruster_ = false; // 追加: スラスター用かどうか

	bool isSmoke_ = false; // 追加: 煙用かどうか

	uint32_t particleCount_ = 1; // 1回の発生で出すパーティクル数（デフォルト1）
};


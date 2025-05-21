#pragma once
#include <string>
#include <Vector3.h>

class ParticleManager;
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

	// 一秒あたりの最大発生数の設定
	void SetParticleRate(uint32_t rate) { particleRate = rate; }

	// 発生場所の取得
	const Vector3& GetPosition() const { return position_; }

	// 一秒あたりの最大発生数の取得
	uint32_t GetParticleRate() const { return particleRate; }

	// リングパーティクルを使うか
	void SetUseRingParticle(bool use) { useRingParticle_ = use; }

	void SetExplosion(bool isExplosion) { isExplosion_ = isExplosion; }
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
	Vector3 position_ = { 0.0f,0.0f,0.0f };

	// パーティクルの一秒あたりの最大発生数
	uint32_t particleRate = 10;

	// パーティクルの発生間隔
	float interval_ = 0.0f;

	// リングパーティクルを使うか
	bool useRingParticle_ = false;

	bool isExplosion_ = false; // 追加: 爆発用かどうか
};


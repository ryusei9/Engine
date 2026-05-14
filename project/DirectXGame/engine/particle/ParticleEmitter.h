#pragma once
#include <string>
#include <Vector3.h>
#include <ParticleType.h>

namespace MyEngine {
	class ParticleManager;

	// ParticleEmitter用の定数
	namespace ParticleEmitterConstants {
		// デルタタイム
		constexpr float kDeltaTime = 1.0f / 60.0f;

		// デフォルトのパーティクル発生レート
		constexpr uint32_t kDefaultParticleRate = 10;

		// デフォルトのパーティクル数
		constexpr uint32_t kDefaultParticleCount = 1;

		// デフォルトの発生間隔
		constexpr float kDefaultInterval = 0.0f;

		// デフォルトの位置
		constexpr float kDefaultPositionX = 0.0f;
		constexpr float kDefaultPositionY = 0.0f;
		constexpr float kDefaultPositionZ = 0.0f;

		// デフォルトの速度
		constexpr float kDefaultVelocityX = 0.0f;
		constexpr float kDefaultVelocityY = 0.0f;
		constexpr float kDefaultVelocityZ = 0.0f;

		// レートのしきい値
		constexpr float kMinValidRate = 0.0f;
	}

	/// <summary>
	/// パーティクル発生器
	/// </summary>
	class ParticleEmitter
	{
	public:
		// コンストラクタ
		ParticleEmitter(ParticleManager* manager, const std::string& groupName);

		// 更新
		void Update();

		// セッター
		void SetPosition(const Vector3& position) { position_ = position; }
		void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }
		void SetParticleRate(uint32_t rate) { particleRate_ = rate; }
		void SetParticleCount(uint32_t count) { particleCount_ = count; }
		void SetParticleType(ParticleType type) { particleType_ = type; }
		

		// ゲッター
		const Vector3& GetPosition() const { return position_; }
		const Vector3& GetVelocity() const { return velocity_; }
		uint32_t GetParticleRate() const { return particleRate_; }
		uint32_t GetParticleCount() const { return particleCount_; }
		float GetInterval() const { return interval_; }
		ParticleType GetParticleType() const { return particleType_; }

	private:
		// パーティクル発生処理
		void EmitParticles();
		void EmitExplosionParticles();

		// 発生間隔の更新
		void UpdateInterval();

		// パーティクル発生判定
		bool ShouldEmitParticles() const;

		// パーティクルマネージャ
		ParticleManager* manager_ = nullptr;

		// パーティクルグループ名
		std::string groupName_;

		// デルタタイム
		float deltaTime_ = ParticleEmitterConstants::kDeltaTime;

		// パーティクルの発生位置
		Vector3 position_ = {
			ParticleEmitterConstants::kDefaultPositionX,
			ParticleEmitterConstants::kDefaultPositionY,
			ParticleEmitterConstants::kDefaultPositionZ
		};

		// パーティクルの速度
		Vector3 velocity_ = {
			ParticleEmitterConstants::kDefaultVelocityX,
			ParticleEmitterConstants::kDefaultVelocityY,
			ParticleEmitterConstants::kDefaultVelocityZ
		};

		// パーティクルの一秒あたりの最大発生数
		uint32_t particleRate_ = ParticleEmitterConstants::kDefaultParticleRate;

		// 1回の発生で出すパーティクル数
		uint32_t particleCount_ = ParticleEmitterConstants::kDefaultParticleCount;

		// パーティクルの発生間隔
		float interval_ = ParticleEmitterConstants::kDefaultInterval;

		ParticleType particleType_ = ParticleType::Normal;
	};
}

#include "ParticleEmitter.h"
#include "ParticleManager.h"
#include <cassert>

//
// ParticleEmitter
// - パーティクルの発生を管理するエミッタークラス
// - 機能：指定された間隔でパーティクルを生成し、ParticleManager に発生リクエストを送る
// - 種類：通常、爆発、スラスター、煙の4種類のパーティクルタイプをサポート
//
namespace MyEngine {
	using namespace ParticleEmitterConstants;

	ParticleEmitter::ParticleEmitter(ParticleManager* manager, const std::string& groupName)
		: manager_(manager)
		, groupName_(groupName)
	{
		assert(manager != nullptr && "ParticleManager pointer is null!");
		assert(!groupName.empty() && "Group name is empty!");
	}

	void ParticleEmitter::Update()
	{
		// 発生間隔を更新
		UpdateInterval();

		// パーティクル発生判定
		if (ShouldEmitParticles()) {
			EmitParticles();
		}
	}

	// ===== ヘルパー関数 =====

	void ParticleEmitter::EmitParticles()
	{

		assert(manager_ != nullptr);

		switch (particleType_)
		{
		case ParticleType::Explosion:
			manager_->EmitExplosion(
				groupName_,
				position_,
				particleCount_
			);
			break;

		case ParticleType::Thruster:
			manager_->EmitWithVelocity(
				groupName_,
				position_,
				particleCount_,
				velocity_,
				ParticleType::Thruster
			);
			break;
		case ParticleType::Smoke:
			manager_->SetIsSmoke(true);
			manager_->EmitWithVelocity(
				groupName_,
				position_,
				particleCount_,
				velocity_,
				ParticleType::Smoke
			);
			break;

		case ParticleType::Charge:
		case ParticleType::Normal:
		case ParticleType::Ring:
		case ParticleType::Cylinder:
			manager_->Emit(
				groupName_,
				position_,
				particleCount_
			);
			break;
		}


		// 発生間隔をリセット
		interval_ -= static_cast<float>(particleRate_) / particleRate_;
	}

	void ParticleEmitter::EmitExplosionParticles()
	{
		assert(manager_ != nullptr && "ParticleManager is null!");
		manager_->EmitExplosion(groupName_, position_, particleCount_);
	}

	void ParticleEmitter::UpdateInterval()
	{
		interval_ += deltaTime_;
	}

	bool ParticleEmitter::ShouldEmitParticles() const
	{
		return particleRate_ > kMinValidRate;
	}
}
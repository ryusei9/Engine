#include "ParticleEmitter.h"
#include "ParticleManager.h"
ParticleEmitter::ParticleEmitter(ParticleManager* manager, const std::string& groupName)
{
	manager_ = manager;
	groupName_ = groupName;

}

void ParticleEmitter::Update()
{
	interval_ += deltaTime_;
	//int particleCount = static_cast<int>(particleRate * interval_);
	if (particleRate > 0.0f) {
		if (isExplosion_) {
			manager_->EmitExplosion(groupName_, position_, particleRate);
		} else {
			manager_->Emit(groupName_, position_, particleRate);
		}
		interval_ -= static_cast<float>(particleRate) / particleRate;
	}
}

#pragma once
#include "PlayerBullet.h"
#include "Collider.h"

class PlayerChargeBullet : public PlayerBullet {
public:
    PlayerChargeBullet();

    void Initialize(const Vector3& position) override;
    void Update() override;
    void Draw() override;

    float GetDamage() const { return damage_; }
    uint32_t GetSerialNumber() const { return serialNumber_; }

private:
    float damage_ = 30.0f;
    uint32_t serialNumber_ = 0;
    static uint32_t nextSerialNumber_;
};
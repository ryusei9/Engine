#pragma once
#include <WorldTransform.h>
#include <memory>
#include <Object3d.h>
#include <Collider.h>
class PlayerLaser : public Collider
{
public:
    void Initialize(const Vector3& start, const Vector3& dir);
    void Update();
    void Draw();

    void SetActive(bool active) { isActive_ = active; }
    bool IsActive() const { return isActive_; }

private:
    bool isActive_ = false;

	uint32_t lifeFrame_ = 360; // レーザーの寿命（フレーム数）

    WorldTransform worldTransform_;
    std::unique_ptr<Object3d> object3d_;

    float length_ = 5.0f; // レーザーの長さ

	Vector3 startPos_;
	Vector3 direction_;
	bool isAlive_ = false;

	// レーザーの移動速度
    float speed_ = 0.2f;

	Vector3 velocity_ = {};

};

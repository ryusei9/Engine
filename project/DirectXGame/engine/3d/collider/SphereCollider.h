#pragma once
#include "Collider.h"
namespace MyEngine {
	class SphereCollider : public Collider
	{
    public:

        ColliderShape GetShape() const override {
            return ColliderShape::Sphere;
        }
	};
}

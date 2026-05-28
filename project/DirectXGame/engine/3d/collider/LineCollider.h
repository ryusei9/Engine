#pragma once
#include "Collider.h"


namespace MyEngine {

	class LineCollider : public Collider
	{
	public:

		void SetStart(const Vector3& start) { start_ = start; }

		void SetEnd(const Vector3& end) { end_ = end; }

		const Vector3& GetStart() const { return start_; }

		const Vector3& GetEnd() const { return end_; }


		Collider::ColliderShape GetShape() const override { return Collider::ColliderShape::Line; }

		Vector3 GetCenterPosition() const override { return (start_ + end_) * 0.5f; }

	private:

		Vector3 start_;
		Vector3 end_;
	};

}
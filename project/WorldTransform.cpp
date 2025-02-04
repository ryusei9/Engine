#include "WorldTransform.h"
#include "MakeIdentity4x4.h"
#include "MakeAffineMatrix.h"
#include "Multiply.h"
void WorldTransform::Initialize()
{
	matWorld_ = MakeIdentity4x4::MakeIdentity4x4();
}
void WorldTransform::UpdateMatrix()
{
	matWorld_ = MakeAffineMatrix::MakeAffineMatrix(scale_, rotate_, translate_);

	if (parent_)
	{
		matWorld_ = Multiply(matWorld_, parent_->matWorld_);
	}
}
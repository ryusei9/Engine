#include "WorldTransform.h"

void WorldTransform::UpdateMatrix() {
	// スケール、回転、平行移動を合成して行列を計算する
	matWorld_ = mathMatrix_.MathMatrix::MakeAffine(scale_, rotation_, translation_);

	// 親があればワールド行列を掛ける
	if (parent_) {
		matWorld_ = mathMatrix_.Multiply(matWorld_,parent_->matWorld_);
	}
	// 定数バッファに転送する
	TransferMatrix();
}
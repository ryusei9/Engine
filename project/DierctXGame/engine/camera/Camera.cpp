#include "Camera.h"
#include <MakeAffineMatrix.h>
#include <Inverse.h>
#include <MakePerspectiveFovMatrix.h>

//
// Camera
// - カメラクラスの更新ロジックを実装したファイル。
// - 役割：ワールド変換（transform_）からビュー/射影行列を生成し、最終的な ViewProjection 行列を得る。
// - 行列の意味：
//     worldMatrix_       : カメラのワールド変換行列（スケール・回転・平行移動を表す）
//     viewMatrix_        : worldMatrix_ の逆行列（ワールド→カメラ空間変換）
//     projectionMatrix_  : 透視投影行列（fovY_, aspectRatio_, nearClip_, farClip_ を使用）
//     viewProjectionMatrix_ : viewMatrix_ * projectionMatrix_（モデル座標→クリップ空間の総合変換）
// - 注意点：
//   ・アスペクト比 (aspectRatio_) はウィンドウリサイズ時に更新する必要がある（ここでは初期化時に WinApp の値を使う）。
//   ・fovY_ はラジアンで扱う想定（MakePerspectiveFovMatrix の契約に従うこと）。
//   ・Update() は transform_ の値（translate/rotate/scale）を変更した後に呼び出すこと。
//   ・行列の掛け合わせ順はレンダリング側のシェーダ期待順に合わせてある（view * projection）。
//

void Camera::Update()
{
	// worldMatrix_ を作成：スケール・回転・平行移動からワールド変換行列を合成する
	worldMatrix_ = MakeAffineMatrix::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	// viewMatrix_ は worldMatrix_ の逆行列（カメラ座標系へ変換）
	viewMatrix_ = Inverse::Inverse(worldMatrix_);

	// projectionMatrix_ は透視投影行列を生成（垂直方向の FOV, アスペクト比, ニア/ファークリップ）
	projectionMatrix_ = MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);

	// 最終的なワールド→クリップ空間変換を用意（描画時に直接使う）
	viewProjectionMatrix_ = Multiply::Multiply(viewMatrix_, projectionMatrix_);
}

Camera::Camera()
	: transform_({{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}})
	, fovY_(0.45f) // 垂直方向の視野角（ラジアン）
	, aspectRatio_(float(WinApp::kClientWidth) / float(WinApp::kClientHeight)) // 初期アスペクト比（ウィンドウサイズ依存）
	, nearClip_(0.1f)
	, farClip_(100.0f)
	// 初期時点で各行列を正しく初期化しておく（transform_ の初期値に基づく）
	, worldMatrix_(MakeAffineMatrix::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate))
	, viewMatrix_(Inverse::Inverse(worldMatrix_))
	, projectionMatrix_(MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_))
	, viewProjectionMatrix_(Multiply::Multiply(viewMatrix_, projectionMatrix_))
{}

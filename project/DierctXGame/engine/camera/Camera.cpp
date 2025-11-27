#include "Camera.h"
#include <MakeAffineMatrix.h>
#include <Inverse.h>
#include <MakePerspectiveFovMatrix.h>

//
// Camera
// - カメラクラスの更新ロジックを実装したファイル。
// - 役割：ワールド変換（transform）からビュー/射影行列を生成し、最終的な ViewProjection 行列を得る。
// - 行列の意味：
//     worldMatrix       : カメラのワールド変換行列（スケール・回転・平行移動を表す）
//     viewMatrix        : worldMatrix の逆行列（ワールド→カメラ空間変換）
//     projectionMatrix  : 透視投影行列（fovY, aspectRatio, nearClip, farClip を使用）
//     viewProjectionMat : viewMatrix * projectionMatrix（モデル座標→クリップ空間の総合変換）
// - 注意点：
//   ・アスペクト比 (aspectRatio) はウィンドウリサイズ時に更新する必要がある（ここでは初期化時に WinApp の値を使う）。
//   ・fovY はラジアンで扱う想定（MakePerspectiveFovMatrix の契約に従うこと）。
//   ・Update() は transform の値（translate/rotate/scale）を変更した後に呼び出すこと。
//   ・行列の掛け合わせ順はレンダリング側のシェーダ期待順に合わせてある（view * projection）。
//

void Camera::Update()
{
	// worldMatrix を作成：スケール・回転・平行移動からワールド変換行列を合成する
	worldMatrix = MakeAffineMatrix::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	// viewMatrix は worldMatrix の逆行列（カメラ座標系へ変換）
	viewMatrix = Inverse::Inverse(worldMatrix);

	// projectionMatrix は透視投影行列を生成（垂直方向の FOV, アスペクト比, ニア/ファークリップ）
	projectionMatrix = MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);

	// 最終的なワールド→クリップ空間変換を用意（描画時に直接使う）
	viewProjectionMatrix = Multiply::Multiply(viewMatrix, projectionMatrix);
}

Camera::Camera()

	: transform({{1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f}})
	, fovY(0.45f) // 垂直方向の視野角（ラジアン）
	, aspectRatio(float(WinApp::kClientWidth) / float(WinApp::kClientHeight)) // 初期アスペクト比（ウィンドウサイズ依存）
	, nearClip(0.1f)
	, farClip(100.0f)
	// 初期時点で各行列を正しく初期化しておく（transform の初期値に基づく）
	, worldMatrix(MakeAffineMatrix::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate))
	, viewMatrix(Inverse::Inverse(worldMatrix))
	, projectionMatrix(MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip))
	, viewProjectionMatrix(Multiply::Multiply(viewMatrix, projectionMatrix))
{}

#pragma once
#include <Matrix4x4.h>
#include <Vector3.h>
#include <wrl.h> // Microsoft::WRL::ComPtrを使用するためのヘッダーファイル
#include <d3d12.h>

class Camera; // 前方宣言

/// <summary>
/// ワールド変換クラス
/// </summary>
class WorldTransform
{
public:
	/*------構造体------*/
	struct TransformationMatrix {
		Matrix4x4 WVP;                    // ワールドビュー射影行列
		Matrix4x4 World;                  // ワールド行列
		Matrix4x4 WorldInversedTranspose; // ワールド逆行列の転置
	};

	/*------メンバ関数------*/

	// 初期化
	void Initialize();

	// 更新
	void Update();

	// パイプラインの設定
	void SetPipeline();

	// BufferResourceの作成（大きい引数はconst参照渡し）
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(
		const Microsoft::WRL::ComPtr<ID3D12Device>& device,
		size_t sizeInBytes);

	// 状態設定（OAOO：同じ処理の使い回し） - ヘッダーにインライン定義
	void SetTransform(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
		scale_ = scale;
		rotate_ = rotate;
		translate_ = translate;
	}
	void SetScale(const Vector3& scale) { scale_ = scale; }
	void SetRotate(const Vector3& rotate) { rotate_ = rotate; }
	void SetTranslate(const Vector3& translate) { translate_ = translate; }

	// 取得（コピー回避のためconst参照を返す） - ヘッダーにインライン定義
	const Vector3& GetScale() const { return scale_; }
	const Vector3& GetRotate() const { return rotate_; }
	const Vector3& GetTranslate() const { return translate_; }

	/*------メンバ変数------*/

	// スケール
	Vector3 scale_ = { 1.0f, 1.0f, 1.0f }; // スケール

	// 回転
	Vector3 rotate_ = { 0.0f, 0.0f, 0.0f }; // 回転

	// 移動
	Vector3 translate_ = { 0.0f, 0.0f, 0.0f }; // 移動

	// ワールド変換行列
	Matrix4x4 matWorld_;

	// 親となるワールド変換
	const WorldTransform* parent_ = nullptr; // 親ワールド変換

private:
	Camera* camera_ = nullptr; // カメラ

	TransformationMatrix* wvpData_ = nullptr; // 変換行列

	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_ = nullptr; // ワールドビュー射影行列バッファ
};


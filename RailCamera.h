#pragma once
#include <WorldTransform.h>
#include <ViewProjection.h>
#include <MathMatrix.h>
/// <summary>
/// レールカメラ
/// </summary>
class RailCamera {
public:
	/// <summary>
	/// デストラクタ
	/// </summary>
	~RailCamera();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Vector3 translation,Vector3 rotate);
	
	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	// ビュー行列を取得
	const Matrix4x4& GetView() { return viewProjection_.matView; }

	// 正射影行列を取得
	const Matrix4x4& GetProjection() { return viewProjection_.matProjection; }

	const WorldTransform& GetWorldTransform() { return worldTransform_; }

private:
	// ワールド変換データ
	WorldTransform worldTransform_;

	// ビュープロジェクション
	ViewProjection viewProjection_;

	// 数学関数
	MathMatrix* mathMatrix_ = nullptr;
};

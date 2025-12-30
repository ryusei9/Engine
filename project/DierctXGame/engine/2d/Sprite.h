#pragma once
#include "Vector4.h"
#include "Vector3.h"
#include "Vector2.h"
#include "DirectXCommon.h"
#include "ModelData.h"
#include "Matrix4x4.h"
#include "MakeIdentity4x4.h"
#include "TextureManager.h"
#include "Material.h"

// 前方宣言
class SpriteCommon;

// 座標変換データ
struct TransformationMatrix {
	Matrix4x4 wvp;
	Matrix4x4 world;
};

/// <summary>
/// スプライト
/// </summary>
class Sprite
{
public:
	// 初期化
	void Initialize(DirectXCommon* dxCommon, std::string textureFilePath);

	// 更新
	void Update();

	// 描画
	void Draw();

	/*------ゲッター------*/

	// スプライト個々の座標の取得
	const Vector2& GetPosition() const { return position_; }

	// スプライト個々の回転角の取得
	float GetRotation() const { return rotation_; }

	// スプライト個々の色の取得
	const Vector4& GetColor() const { return materialData_->color; }

	// スプライト個々のサイズの取得
	const Vector2& GetSize() const { return size_; }

	// アンカーポイントの取得
	const Vector2& GetAnchorPoint() const { return anchorPoint_; }

	// 左右フリップの取得
	bool GetIsFlipX() const { return isFlipX_; }

	// 上下フリップの取得
	bool GetIsFlipY() const { return isFlipY_; }

	// テクスチャ左上座標の取得
	const Vector2& GetTextureLeftTop() const { return textureLeftTop_; }

	// テクスチャ切り出しサイズの取得
	const Vector2& GetTextureSize() const { return textureSize_; }

	/*------セッター------*/

	// スプライト個々の座標の設定
	void SetPosition(const Vector2& position) { position_ = position; }

	// スプライト個々の回転角の設定
	void SetRotation(float rotation) { rotation_ = rotation; }

	// スプライト個々の色の設定
	void SetColor(const Vector4& color) { materialData_->color = color; }

	// スプライト個々のサイズの設定
	void SetSize(const Vector2& size) { size_ = size; }

	// アンカーポイントの設定
	void SetAnchorPoint(const Vector2& anchorPoint) { anchorPoint_ = anchorPoint; }

	// 左右フリップの設定
	void SetIsFlipX(bool isFlipX) { isFlipX_ = isFlipX; }

	// 上下フリップの設定
	void SetIsFlipY(bool isFlipY) { isFlipY_ = isFlipY; }

	// テクスチャ左上座標の設定
	void SetTextureLeftTop(const Vector2& textureLeftTop) { textureLeftTop_ = textureLeftTop; }

	// テクスチャ切り出しサイズの設定
	void SetTextureSize(const Vector2& textureSize) { textureSize_ = textureSize; }

private:
	// BufferResourceの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes);
	void CreateVertexData();
	void CreateMaterialData();
	void CreateWVPData();

	// テクスチャサイズをイメージに合わせる
	void AdjustTextureSize();

	SpriteCommon* spriteCommon_ = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;

	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;
	TransformationMatrix* transformationMatrixData_ = nullptr;

	// バッファリソースの使い道を補足するバッファビュー
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	DirectXCommon* dxCommon_ = nullptr;

	Material* materialData_ = nullptr;

	// スプライト個々の座標
	Vector2 position_ = { 0.0f, 0.0f };

	// スプライト個々の回転角
	float rotation_ = 0.0f;

	// スプライト個々のサイズ
	Vector2 size_ = { 640.0f, 360.0f };

	// テクスチャ番号
	uint32_t textureIndex_ = 0;

	// アンカーポイント
	Vector2 anchorPoint_ = { 0.0f, 0.0f };

	// 左右フリップ
	bool isFlipX_ = false;

	// 上下フリップ
	bool isFlipY_ = false;

	// テクスチャ左上座標
	Vector2 textureLeftTop_ = { 0.0f, 0.0f };

	// テクスチャ切り出しサイズ
	Vector2 textureSize_ = { 100.0f, 100.0f };

	// テクスチャファイルパス
	std::string filePath_;
};



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

// 頂点データ

// 座標変換データ
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};
/// <summary>
/// スプライト
/// </summary>
class Sprite
{
public:
	// 初期化
	void Initialize(DirectXCommon* dxCommon,std::string textureFilePath);

	// 更新
	void Update();

	// 描画
	void Draw();

	/*------ゲッター------*/

	// スプライト個々の座標の取得
	const Vector2& GetPosition() const { return position; }

	// スプライト個々の回転角の取得
	float GetRotation()const { return rotation; }

	// スプライト個々の色の取得
	const Vector4& GetColor()const { return materialData->color; }

	// スプライト個々のサイズの取得
	const Vector2& GetSize() const { return size; }

	// アンカーポイントの取得
	const Vector2& GetAnchorPoint() const { return anchorPoint; }

	// 左右フリップの取得
	const bool& GetIsFlipX() const { return isFlipX_; }

	// 上下フリップの取得
	const bool& GetIsFlipY() const { return isFlipY_; }

	// テクスチャ左上座標の取得
	const Vector2& GetTextureLeftTop()const { return textureLeftTop; }

	// テクスチャ切り出しサイズの取得
	const Vector2& GetTextureSize()const { return textureSize; }


	/*------セッター------*/

	// スプライト個々の座標の設定
	void SetPosition(const Vector2& position) { this->position = position; }

	// スプライト個々の回転角の設定
	void SetRotation(float rotation) { this->rotation = rotation; }

	// スプライト個々の色の設定
	void SetColor(const Vector4& color) { materialData->color = color; }

	// スプライト個々のサイズの設定
	void SetSize(const Vector2& size) { this->size = size; }

	// アンカーポイントの設定
	void SetAnchorPoint(const Vector2& anchorPoint) { this->anchorPoint = anchorPoint; }

	// 左右フリップの設定
	void SetIsFlipX(const bool& isFlipX_) { this->isFlipX_ = isFlipX_; }

	// 上下フリップの設定
	void SetIsFlipY(const bool& isFlipY_) { this->isFlipY_ = isFlipY_; }

	// テクスチャ左上座標の設定
	void SetTextureLeftTop(const Vector2& textureLeftTop) { this->textureLeftTop = textureLeftTop; }

	// テクスチャ切り出しサイズの設定
	void SetTextureSize(const Vector2& textureSize) { this->textureSize = textureSize; }
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
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;

	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	TransformationMatrix* transformationMatrixData = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	DirectXCommon* dxCommon_ = nullptr;

	Material* materialData = nullptr;

	// スプライト個々の座標
	Vector2 position = { 0.0f,0.0f };

	// スプライト個々の回転角
	float rotation = 0.0f;

	// スプライト個々のサイズ
	Vector2 size = { 640.0f,360.0f };

	// テクスチャ番号
	uint32_t textureIndex = 0;

	Vector2 anchorPoint = { 0.0f,0.0f };

	// 左右フリップ
	bool isFlipX_ = false;

	// 上下フリップ
	bool isFlipY_ = false;

	// テクスチャ左上座標
	Vector2 textureLeftTop = { 0.0f,0.0f };

	// テクスチャ切り出しサイズ
	Vector2 textureSize = { 100.0f,100.0f };

	std::string filePath; // 追加: テクスチャファイルパスを格納するためのメンバー変数
};




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
// スプライト
class Sprite
{
public:
	// 初期化
	void Initialize(DirectXCommon* dxCommon,std::string textureFilePath);

	// 更新
	void Update();

	void Draw();

	// getter
	const Vector2& GetPosition() const { return position; }

	float GetRotation()const { return rotation; }

	const Vector4& GetColor()const { return materialData->color; }

	const Vector2& GetSize() const { return size; }

	const Vector2& GetAnchorPoint() const { return anchorPoint; }

	const bool& GetIsFlipX() const { return isFlipX_; }

	const bool& GetIsFlipY() const { return isFlipY_; }

	const Vector2& GetTextureLeftTop()const { return textureLeftTop; }

	const Vector2& GetTextureSize()const { return textureSize; }


	// setter
	void SetPosition(const Vector2& position) { this->position = position; }

	void SetRotation(float rotation) { this->rotation = rotation; }

	void SetColor(const Vector4& color) { materialData->color = color; }

	void SetSize(const Vector2& size) { this->size = size; }

	void SetAnchorPoint(const Vector2& anchorPoint) { this->anchorPoint = anchorPoint; }

	void SetIsFlipX(const bool& isFlipX_) { this->isFlipX_ = isFlipX_; }

	void SetIsFlipY(const bool& isFlipY_) { this->isFlipY_ = isFlipY_; }

	void SetTextureLeftTop(const Vector2& textureLeftTop) { this->textureLeftTop = textureLeftTop; }

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




#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <WorldTransform.h>
#include <Camera.h>
#include <Vector2.h>
#include <Vector4.h>
#include <string>

// Skybox用の定数
namespace SkyboxConstants {
	// インデックス数
	constexpr uint32_t kNumIndices = 36; // 6面の立方体、各面4頂点、2つの三角形で構成
	
	// 頂点数
	constexpr uint32_t kNumVertices = 24; // 6面 × 4頂点
	constexpr uint32_t kVerticesPerFace = 4;
	constexpr uint32_t kNumFaces = 6;
	
	// デフォルト値
	constexpr float kDefaultScale = 50.0f;
	constexpr float kDefaultRotation = 0.0f;
	constexpr float kDefaultTranslateZ = -10.0f;
	constexpr float kDefaultColorR = 1.0f;
	constexpr float kDefaultColorG = 1.0f;
	constexpr float kDefaultColorB = 1.0f;
	constexpr float kDefaultColorA = 1.0f;
	
	// ルートパラメータインデックス
	constexpr uint32_t kRootParameterIndexMaterial = 0;
	constexpr uint32_t kRootParameterIndexTransformation = 1;
	constexpr uint32_t kRootParameterIndexTexture = 2;
	constexpr uint32_t kRootParameterCount = 3;
	
	// シェーダーレジスタ番号
	constexpr uint32_t kMaterialRegister = 0;
	constexpr uint32_t kTransformationRegister = 0;
	constexpr uint32_t kTextureRegister = 0;
	constexpr uint32_t kSamplerRegister = 0;
	
	// シェーダーパス
	constexpr const wchar_t* kVertexShaderPath = L"resources/shaders/Skybox.VS.hlsl";
	constexpr const wchar_t* kPixelShaderPath = L"resources/shaders/Skybox.PS.hlsl";
	constexpr const wchar_t* kVertexShaderProfile = L"vs_6_0";
	constexpr const wchar_t* kPixelShaderProfile = L"ps_6_0";
	
	// 入力レイアウトの要素数
	constexpr uint32_t kInputElementCount = 2;
}

/// <summary>
/// スカイボックス
/// </summary>
class Skybox
{
public:
	// 頂点データ構造体
	struct Vertex {
		Vector4 position;
		Vector3 texcoord;
	};

	// マテリアル構造体
	struct Material {
		Vector4 color; // 色
		Matrix4x4 uvTransform; // UV変換行列
		float padding[3]; // パディング
	};
   
	// 初期化
	void Initialize(const std::string& texturePath);

	// 更新
	void Update();

	// 描画
	void Draw();

	// ImGui描画
	void DrawImGui();

	// スカイボックス描画準備
	void DrawSettings();

	// ゲッター
	WorldTransform& GetWorldTransform() { return worldTransform_; }
	std::string GetFilePath() const { return filePath_; }

	// セッター
	void SetCamera(Camera* camera);

private:
	// 頂点バッファ作成
	void CreateVertexBuffer();

	// テクスチャ作成
	void CreateTexture(const std::string& texturePath);

	// マテリアルリソース作成
	void CreateMaterialResource();

	// ルートシグネチャ作成
	void CreateRootSignature();

	// パイプラインステート作成
	void CreatePipelineState();

	// インデックスバッファ作成
	void CreateIndexBuffer();

	// ヘルパー関数
	void SetupInputLayout();
	void SetupBlendState();
	void SetupRasterizerState();
	void SetupDepthStencilState();
	void SetupRootParameters(D3D12_ROOT_PARAMETER* rootParameters, D3D12_DESCRIPTOR_RANGE* descriptorRange);
	void SetupStaticSampler(D3D12_STATIC_SAMPLER_DESC* samplerDesc);
	void InitializeVertexData();
	void InitializeIndexData();
	
	// 頂点バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};
	std::vector<Vertex> vertices_;

	// テクスチャ
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_;
	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle_{};

	// マテリアルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Material* materialData_ = nullptr;

	// カメラ
	Camera* camera_ = nullptr;

	// ワールド変換
	WorldTransform worldTransform_;

	// ルートシグネチャとパイプライン
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_ = nullptr;
	
	// 入力レイアウト
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_{};
	D3D12_INPUT_ELEMENT_DESC inputElementDescs_[SkyboxConstants::kInputElementCount] = {};
	
	// レンダーステート
	D3D12_RENDER_TARGET_BLEND_DESC blendDesc_{};
	D3D12_RASTERIZER_DESC rasterizerDesc_{};
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};

	// インデックスバッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
	uint32_t* indexData_ = nullptr;

	// ファイルパス
	std::string filePath_;
};
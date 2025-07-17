#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <WorldTransform.h>
#include <Camera.h>
#include <Vector2.h>
#include <Vector4.h>
#include <string>
class Skybox
{
public:
    struct Vertex {
        Vector4 position;
        Vector3 texcoord;
    };
    struct Material {
		Vector4 color; // 色
		Matrix4x4 uvTransform; // UV変換行列
		float padding[3]; // パディング
    };
   

    void Initialize(const std::string& texturePath);

    void Update();

    void Draw();

	void DrawImGui();

    void SetCamera(Camera* camera);

	WorldTransform& GetWorldTransform() { return worldTransform_; }

private:
    void CreateVertexBuffer();
    void CreateTexture(const std::string& texturePath);
    void CreateMaterialResource();
    void CreateRootSignature();
    void CreatePipelineState();
	void CreateIndexBuffer();

	const UINT kNumIndex = 36; // 6面の立方体、各面4頂点、2つの三角形で構成
	
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
    D3D12_VERTEX_BUFFER_VIEW vbView_{};
    std::vector<Vertex> vertices_;

    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_;
    D3D12_GPU_DESCRIPTOR_HANDLE textureHandle_{};

    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
    
    Material* materialData_ = nullptr;

    Camera* camera_ = nullptr;

	WorldTransform worldTransform_;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_ = nullptr;
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_{};
    D3D12_INPUT_ELEMENT_DESC inputElementDescs_[2] = {};
    D3D12_RENDER_TARGET_BLEND_DESC blendDesc_{};
    D3D12_RASTERIZER_DESC rasterizerDesc_{};
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};

    Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
    D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
    uint32_t* indexData_ = nullptr;
};
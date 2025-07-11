#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <WorldTransform.h>
#include <Camera.h>
#include <Vector2.h>
#include <Vector4.h>

class Skybox
{
public:
    struct Vertex {
        Vector4 position;
        Vector3 normal;
        Vector2 texcoord;
    };
    struct CameraMatrix {
        Matrix4x4 viewProj;
    };
   

    void Initialize(const std::string& texturePath);

    void Update();

    void Draw();

    void SetCamera(Camera* camera);

	WorldTransform& GetWorldTransform() { return worldTransform_; }

private:
    void CreateVertexBuffer();
    void CreateTexture(const std::string& texturePath);
    void CreateCameraBuffer();
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
    D3D12_VERTEX_BUFFER_VIEW vbView_{};
    std::vector<Vertex> vertices_;

    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_;
    D3D12_GPU_DESCRIPTOR_HANDLE textureHandle_{};

    Microsoft::WRL::ComPtr<ID3D12Resource> cameraBuffer_;
    
    CameraMatrix* cameraData_ = nullptr;

    Camera* camera_ = nullptr;

	WorldTransform worldTransform_;
};
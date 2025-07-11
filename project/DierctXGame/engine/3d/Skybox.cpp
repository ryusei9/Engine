#include "Skybox.h"
#include <cassert>
#include <cstring>
#include <DirectXCommon.h>
#include <TextureManager.h>
#include <MakeAffineMatrix.h>


void Skybox::Initialize(const std::string& texturePath)
{
    // 8頂点（各面4頂点ずつ、インデックスで面を作る場合は24頂点でもOK）
    // ここでは各面ごとに4頂点ずつ定義（内側向き）
    vertices_ = {
        // 右面 (+X)
        {{ 1.0f,  1.0f,  1.0f, 1.0f }, {}, {}},
        {{ 1.0f,  1.0f, -1.0f, 1.0f }, {}, {}},
        {{ 1.0f, -1.0f, -1.0f, 1.0f }, {}, {}},
        {{ 1.0f, -1.0f,  1.0f, 1.0f }, {}, {}},
        // 左面 (-X)
        {{-1.0f,  1.0f, -1.0f, 1.0f }, {}, {}},
        {{-1.0f,  1.0f,  1.0f, 1.0f }, {}, {}},
        {{-1.0f, -1.0f,  1.0f, 1.0f }, {}, {}},
        {{-1.0f, -1.0f, -1.0f, 1.0f }, {}, {}},
        // 前面 (+Z)
        {{-1.0f,  1.0f,  1.0f, 1.0f }, {}, {}},
        {{ 1.0f,  1.0f,  1.0f, 1.0f }, {}, {}},
        {{ 1.0f, -1.0f,  1.0f, 1.0f }, {}, {}},
        {{-1.0f, -1.0f,  1.0f, 1.0f }, {}, {}},
        // 背面 (-Z)
        {{ 1.0f,  1.0f, -1.0f, 1.0f }, {}, {}},
        {{-1.0f,  1.0f, -1.0f, 1.0f }, {}, {}},
        {{-1.0f, -1.0f, -1.0f, 1.0f }, {}, {}},
        {{ 1.0f, -1.0f, -1.0f, 1.0f }, {}, {}},
        // 上面 (+Y)
        {{-1.0f,  1.0f, -1.0f, 1.0f }, {}, {}},
        {{ 1.0f,  1.0f, -1.0f, 1.0f }, {}, {}},
        {{ 1.0f,  1.0f,  1.0f, 1.0f }, {}, {}},
        {{-1.0f,  1.0f,  1.0f, 1.0f }, {}, {}},
        // 下面 (-Y)
        {{-1.0f, -1.0f,  1.0f, 1.0f }, {}, {}},
        {{ 1.0f, -1.0f,  1.0f, 1.0f }, {}, {}},
        {{ 1.0f, -1.0f, -1.0f, 1.0f }, {}, {}},
        {{-1.0f, -1.0f, -1.0f, 1.0f }, {}, {}},
    };

    CreateVertexBuffer();
    CreateTexture(texturePath);
    CreateCameraBuffer();

    worldTransform_.scale_ = { 1.0f, 1.0f, 1.0f };
    worldTransform_.rotate_ = { 0.0f, 0.0f, 0.0f };
    worldTransform_.translate_ = { 0.0f, 0.0f, 0.0f };

}

void Skybox::CreateVertexBuffer()
{
    const UINT size = static_cast<UINT>(sizeof(Vertex) * vertices_.size());

    D3D12_HEAP_PROPERTIES heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = size;
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    HRESULT hr = DirectXCommon::GetInstance()->GetDevice()->CreateCommittedResource(
        &heapProp, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&vertexBuffer_));
    assert(SUCCEEDED(hr));

    void* mapped = nullptr;
    vertexBuffer_->Map(0, nullptr, &mapped);
    std::memcpy(mapped, vertices_.data(), size);
    vertexBuffer_->Unmap(0, nullptr);

    vbView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
    vbView_.SizeInBytes = size;
    vbView_.StrideInBytes = sizeof(Vertex);
}

void Skybox::CreateTexture(const std::string& texturePath)
{
    // TextureManager経由でSRVを取得
    TextureManager* texMgr = TextureManager::GetInstance().get();
    texMgr->LoadTexture(texturePath);
    textureHandle_ = texMgr->GetSrvHandleGPU(texturePath);
}

void Skybox::SetCamera(Camera* camera)
{
    camera_ = camera;
}

void Skybox::CreateCameraBuffer()
{
    auto* dxCommon = DirectXCommon::GetInstance();
    cameraBuffer_ = dxCommon->CreateBufferResource(sizeof(CameraMatrix));
    cameraBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));
}

void Skybox::Update()
{
    // WorldTransform行列を作成
    Matrix4x4 world = MakeAffineMatrix::MakeAffineMatrix(
        worldTransform_.scale_,
        worldTransform_.rotate_,
        worldTransform_.translate_
    );

    // カメラ行列を取得
    Matrix4x4 view = camera_ ? camera_->GetViewMatrix() : Matrix4x4();
    Matrix4x4 proj = camera_ ? camera_->GetProjectionMatrix() : Matrix4x4();

    // WVPを計算してCBVに書き込む
    if (cameraData_) {
        cameraData_->viewProj = world * view * proj;
    }
}

void Skybox::Draw()
{
    auto* cmdList = DirectXCommon::GetInstance()->GetCommandList();

    // VBVセット
    cmdList->IASetVertexBuffers(0, 1, &vbView_);

    // CBVとテクスチャSRVをセット（RootParameterIndexはプロジェクトに合わせて調整）
    cmdList->SetGraphicsRootConstantBufferView(0, cameraBuffer_->GetGPUVirtualAddress());
    cmdList->SetGraphicsRootDescriptorTable(1, textureHandle_);

    // 6面分描画
    for (int i = 0; i < 6; ++i) {
        cmdList->DrawInstanced(4, 1, i * 4, 0);
    }
}
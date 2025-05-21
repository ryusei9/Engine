#include "WorldTransform.h"
#include <Object3dCommon.h>
#include <Object3d.h>
#include <MakeIdentity4x4.h>
#include <MakeAffineMatrix.h>
#include <Inverse.h>
void WorldTransform::Initialize()
{
	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();

	// WVP用のリソースを作る
	wvpResource_ = CreateBufferResource(Object3dCommon::GetInstance()->GetDxCommon()->GetDevice(),sizeof(TransformationMatrix));

	// WVP用のリソースの設定
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	// 単位行列
	wvpData_->WVP = MakeIdentity4x4::MakeIdentity4x4();
	// ワールド行列
	wvpData_->World = MakeIdentity4x4::MakeIdentity4x4();
	// ビュー行列
	wvpData_->WorldInversedTranspose = MakeIdentity4x4::MakeIdentity4x4();
}

void WorldTransform::Update()
{
	Matrix4x4 worldMatrix = MakeAffineMatrix::MakeAffineMatrix(scale_, rotate_, translate_);
	Matrix4x4 worldViewProjectionMatrix;

	// 親オブジェクトがあれば親のワールド行列を掛ける
	if (parent_)
	{
		worldMatrix = Multiply::Multiply(worldMatrix, parent_->matWorld_);
	}

	if (camera_)
	{
		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply::Multiply(worldMatrix, viewProjectionMatrix);
	} else
	{
		worldViewProjectionMatrix = worldMatrix;
	}

	wvpData_->WVP = worldViewProjectionMatrix;
	wvpData_->World = worldMatrix;
	wvpData_->WorldInversedTranspose = Matrix4x4::Transpose(Inverse::Inverse(worldMatrix));
}

void WorldTransform::SetPipeline()
{
	auto commandList = Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList();

	commandList->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
}

Microsoft::WRL::ComPtr<ID3D12Resource> WorldTransform::CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes)
{
	// DXGIファクトリーの生成
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	// 関数が成功したかどうか
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};

	// UploadHeapを使う
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	// 頂点リソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};

	// バッファリソース。テクスチャの場合はまた別の設定をする
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	// リソースのサイズ
	resourceDesc.Width = sizeInBytes;

	// バッファの場合はこれらは1にする決まり
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;

	// バッファの場合はこれにする決まり
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;



	// 実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

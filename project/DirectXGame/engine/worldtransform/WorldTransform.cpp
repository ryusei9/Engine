#include "WorldTransform.h"
#include <Object3dCommon.h>
#include <Object3d.h>
#include <MakeIdentity4x4.h>
#include <MakeAffineMatrix.h>
#include <Inverse.h>
#include <ResourceManager.h>
namespace MyEngine {
	using namespace Math;

	void WorldTransform::Initialize()
	{
		camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();

		// WVP用のリソースを作る
		wvpResource_ = ResourceManager::CreateBufferResource(
			Object3dCommon::GetInstance()->GetDxCommon()->GetDevice().Get(),
			sizeof(TransformationMatrix));

		// WVP用のリソースの設定
		wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
		// 単位行列
		wvpData_->WVP = MakeIdentity4x4::MakeIdentity4x4();
		// ワールド行列
		wvpData_->World = MakeIdentity4x4::MakeIdentity4x4();
		// ワールド逆行列の転置
		wvpData_->WorldInversedTranspose = MakeIdentity4x4::MakeIdentity4x4();
	}

	void WorldTransform::Update()
	{
		Matrix4x4 worldMatrix = MakeAffineMatrix::MakeAffineMatrix(scale_, rotate_, translate_);
		Matrix4x4 worldViewProjectionMatrix;

		// 親オブジェクトがあれば親のワールド行列を掛ける
		if (parent_)
		{
			worldMatrix = Multiply(worldMatrix, parent_->matWorld_);
		}

		if (camera_)
		{
			const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
			worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
		}
		else
		{
			worldViewProjectionMatrix = worldMatrix;
		}

		wvpData_->WVP = worldViewProjectionMatrix;
		wvpData_->World = worldMatrix;
		wvpData_->WorldInversedTranspose = Matrix4x4::Transpose(Inverse(worldMatrix));
	}

	void WorldTransform::SetPipeline()
	{
		auto commandList = Object3dCommon::GetInstance()->GetDxCommon()->GetCommandList();
		commandList->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	}
}

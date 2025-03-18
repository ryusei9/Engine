#pragma once
#include <DirectXCommon.h>
#include <Camera.h>
#include <SrvManager.h>
/*------パーティクルを管理するクラス------*/
class ParticleManager
{
public:
	/*------メンバ関数------*/
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);

private:
	void CreateRootSignature();
	/*------メンバ変数------*/
	DirectXCommon* dxCommon_ = nullptr;

	SrvManager* srvManager_ = nullptr;


};


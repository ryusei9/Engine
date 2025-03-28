#pragma once
#include <DirectXCommon.h>
#include <Camera.h>
#include <SrvManager.h>
#include <MaterialData.h>
#include <Vector4.h>
#include <ModelData.h>
#include <random>
class Camera;
/*------パーティクルを管理するクラス------*/
class ParticleManager
{
public:
	/*------構造体------*/
	// パーティクルの構造体
	struct Particle {
		Transform transform;
		Vector3 velocity;
		Vector4 color;
		float lifeTime;
		float currentTime;
	};

	struct ParticleForGPU {
		Matrix4x4 WVP;
		Matrix4x4 World;
		Vector4 color;
	};

	struct AABB {
		Vector3 min;
		Vector3 max;
	};

	struct AccelerationField {
		Vector3 acceleration;	// 加速度
		AABB area;	// 範囲
	};

	struct WindZone{
		AABB area;		  // 風が吹くエリア
		Vector3 strength; // 風の強さ
	};

	struct Emitter {
		Transform transform;
		uint32_t count;
		float frequency;	// 発生頻度
		float frequencyTime;	// 頻度用時刻
	};

	// パーティクルグループの構造体
	struct ParticleGroup {
		// マテリアルデータ
		MaterialData materialData;
		// パーティクルのリスト
		std::list<Particle> particles;
		// インスタンシングデータ用SRVインデックス
		uint32_t srvIndex;
		// インスタンシングリソース
		ParticleForGPU* instanceData;
		// インスタンス数
		uint32_t numParticles = 0;
		// インスタンシングデータを書き込むためのポインタ
		Microsoft::WRL::ComPtr<ID3D12Resource> instanceBuffer;
	};

	/*------メンバ関数------*/

	// シングルトンインスタンス
	static std::shared_ptr<ParticleManager> GetInstance();

	ParticleManager() = default;
	~ParticleManager() = default;
	ParticleManager(const ParticleManager&) = delete;
	ParticleManager& operator=(const ParticleManager&) = delete;

	// 初期化
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager,Camera* camera);

	// 更新
	void Update();

	// 描画
	void Draw();

	// 終了
	void Finalize();

	// パーティクルグループの追加
	void CreateParticleGroup(const std::string& name, const std::string textureFilePath);

	// バッファリソースの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);

	// パーティクルの発生場所を設定
	void Emit(const std::string name, const Vector3& position, uint32_t count);

	// パーティクルの生成
	Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate);

	/*------頂点データの作成------*/
	void CreateVertexData();

	/*------マテリアルデータの作成------*/
	void CreateMaterialData();
private:
	static std::shared_ptr<ParticleManager> instance;

	void CreateRootSignature();

	void CreatePSO();

	// AABBとVector3の当たり判定
	bool IsCollision(const AABB& aabb, const Vector3& point);

	

	/*------メンバ変数------*/
	DirectXCommon* dxCommon_ = nullptr;

	SrvManager* srvManager_ = nullptr;

	/*------ルートシグネイチャ------*/
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	ModelData modelData;
	// 頂点データ
	VertexData* vertexData;

	/*------生成する上限------*/
	const uint32_t kNumMaxInstance = 100;

	Camera* camera_ = nullptr;

	const float kDeltaTime = 1.0f / 60.0f;

	bool useBillboard = false;

	bool isWind = false;

	// 加速度フィールド
	AccelerationField accelerationField;

	std::vector<ParticleManager::WindZone> windZones = {
	{ { {-5.0f, -5.0f, -5.0f}, {5.0f, 5.0f, 5.0f} }, {0.1f, 0.0f, 0.0f} },
	{ { {10.0f, -5.0f, -5.0f}, {15.0f, 5.0f, 5.0f} }, {0.0f, 0.0f, 0.1f} }
	};
	// ランダム
	std::random_device seedGeneral;
	std::mt19937 randomEngine;

	std::unordered_map<std::string, ParticleGroup> particleGroups;
};


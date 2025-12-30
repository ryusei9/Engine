#pragma once
#include <DirectXCommon.h>
#include <Camera.h>
#include <SrvManager.h>
#include <MaterialData.h>
#include <Vector4.h>
#include <ModelData.h>
#include <random>
#include "Material.h"
#include <ParticleType.h>

class DirectXCommon;// 前方宣言
class Camera;// 前方宣言
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
		bool isExplosion = false;	// 爆発パーティクルかどうか
		bool isSubExplosion = false; // 追加: サブパーティクル用
		float maxScale = 1.0f; // 追加: 最大スケール
	};

	// GPU用パーティクル構造体
	struct ParticleForGPU {
		Matrix4x4 WVP;
		Matrix4x4 World;
		Vector4 color;
	};

	// 軸合わせ用AABB構造体
	struct AABB {
		Vector3 min;
		Vector3 max;
	};

	// 加速度フィールドの構造体
	struct AccelerationField {
		Vector3 acceleration;	// 加速度
		AABB area;	// 範囲
	};

	// 風エリアの構造体
	struct WindZone{
		AABB area;		  // 風が吹くエリア
		Vector3 strength; // 風の強さ
	};

	// エミッターの構造体
	struct Emitter {
		Transform transform;
		uint32_t count;
		float frequency;	// 発生頻度
		float frequencyTime;	// 頻度用時刻
		std::string groupName;
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
	static ParticleManager* GetInstance();

	ParticleManager() = default;
	~ParticleManager() = default;
	ParticleManager(const ParticleManager&) = delete;
	ParticleManager& operator=(const ParticleManager&) = delete;

	// 初期化
	void Initialize(SrvManager* srvManager, Camera* camera);

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

	// 爆発パーティクルの発生場所を設定
	void EmitExplosion(const std::string& name, const Vector3& position, uint32_t count);

	void EmitWithVelocity(const std::string& name, const Vector3& position, uint32_t count, const Vector3& velocity);

	// パーティクルの生成
	Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate);

	// プレーンパーティクルの生成
	Particle MakeNewPlaneParticle(std::mt19937& randomEngine, const Vector3& translate);

	// リングパーティクルの生成
	Particle MakeNewRingParticle(std::mt19937& randomEngine, const Vector3& translate);

	// 円柱パーティクルの生成
	Particle MakeNewCylinderParticle(std::mt19937& randomEngine, const Vector3& translate);

	// スラスターパーティクルの生成
	Particle MakeNewThrusterParticle(std::mt19937& randomEngine, const Vector3& translate);

	// 煙パーティクルの生成
	Particle MakeNewSmokeParticle(std::mt19937& randomEngine, const Vector3& translate);

	void UpdateExplosionParticle(Particle& particle);

	/*------頂点データの作成------*/
	void CreateVertexData();

	// リング用の頂点データの作成
	void CreateRingVertexData();

	// 円柱用の頂点データの作成
	void CreateCylinderVertexData();

	/*------マテリアルデータの作成------*/
	void CreateMaterialData();

	// ImGuiの描画
	void DrawImGui();

	/*------ゲッター------*/
	// ビルボードの取得
	bool GetUseBillboard() const { return useBillboard_; }

	// リング型の頂点を使うか
	bool GetUseRingVertex() const { return useRingVertex_; }

	// パーティクルタイプの取得
	ParticleType GetParticleType() const { return particleType_; }

	/*------セッター------*/
	// ビルボードの設定
	void SetUseBillboard(bool useBillboard) { useBillboard_ = useBillboard; }

	// リング型の頂点を使うか
	void SetUseRingVertex(bool useRingVertex) { useRingVertex_ = useRingVertex; }

	// パーティクルタイプの設定
	void SetParticleType(ParticleType type);

	// スケールの設定
	void SetParticleScale(const Vector3& scale) { uvTransform_.scale = scale; }

	void SetIsSmoke(bool isSmoke) { isSmoke_ = isSmoke; }

private:
	// ルートシグネイチャの作成
	void CreateRootSignature();

	// パイプラインステートオブジェクトの作成
	void CreatePSO();

	// AABBとVector3の当たり判定
	bool IsCollision(const AABB& aabb, const Vector3& point);

	ParticleType particleType_ = ParticleType::Normal;

	/*------メンバ変数------*/
	DirectXCommon* dxCommon_ = nullptr;

	SrvManager* srvManager_ = nullptr;

	/*------ルートシグネイチャ------*/
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	ModelData modelData_;
	// 頂点データ
	VertexData* vertexData_;

	/*------生成する上限------*/
	const uint32_t kNumMaxInstance_ = 100;

	Camera* camera_ = nullptr;

	const float kDeltaTime_ = 1.0f / 60.0f;

	bool useBillboard_ = false;

	bool isWind_ = false;

	// 加速度フィールド
	AccelerationField accelerationField_;

	std::vector<ParticleManager::WindZone> windZones_ = {
		{ { {-5.0f, -5.0f, -5.0f}, {5.0f, 5.0f, 5.0f} }, {0.1f, 0.0f, 0.0f} },
		{ { {10.0f, -5.0f, -5.0f}, {15.0f, 5.0f, 5.0f} }, {5.0f, 0.0f, 0.0f} }
	};
	// ランダム
	std::random_device seedGeneral_;
	std::mt19937 randomEngine_;

	std::unordered_map<std::string, ParticleGroup> particleGroups_;

	// リング型の頂点を使うか
	bool useRingVertex_ = false;

	Transform uvTransform_{
		{1.0f, 1.0f, 1.0f}, // スケール
		{0.0f, 0.0f, 0.0f}, // 回転
		{0.0f, 0.0f, 0.0f}  // 座標
	};

	Material* materialData_ = nullptr;

	bool isSmoke_ = false;
};


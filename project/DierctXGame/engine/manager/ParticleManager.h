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
#include <string>
#include <cstdint>

// 前方宣言
class DirectXCommon;
class Camera;

/// <summary>
/// ParticleManager用の定数
/// </summary>
namespace ParticleManagerConstants {
	// インスタンシング
	constexpr uint32_t kMaxInstanceCount = 100;
	
	// デルタタイム
	constexpr float kDeltaTime = 1.0f / 60.0f;
	
	// リング頂点生成
	constexpr uint32_t kRingDivision = 32;
	constexpr float kRingOuterRadius = 1.0f;
	constexpr float kRingInnerRadius = 0.2f;
	
	// 円柱頂点生成
	constexpr uint32_t kCylinderDivision = 32;
	constexpr float kCylinderTopRadius = 1.0f;
	constexpr float kCylinderBottomRadius = 1.0f;
	constexpr float kCylinderHeight = 3.0f;
	
	// パーティクル生成範囲
	constexpr float kParticleSpawnRangeMin = -5.0f;
	constexpr float kParticleSpawnRangeMax = 5.0f;
	
	// パーティクル生存時間
	constexpr float kParticleLifeTimeMin = 1.0f;
	constexpr float kParticleLifeTimeMax = 3.0f;
	constexpr float kThrusterLifeTime = 0.1f;
	constexpr float kSmokeLifeTime = 0.1f;
	
	// パーティクルサイズ
	constexpr float kDefaultParticleScale = 1.0f;
	constexpr float kThrusterParticleScale = 0.3f;
	constexpr float kSmokeParticleScale = 0.3f;
	
	// 爆発パーティクル
	constexpr float kExplosionCenterLifeTime = 1.0f;
	constexpr float kExplosionCenterMaxScale = 1.25f;
	constexpr float kExplosionSubLifeTimeMin = 1.2f;
	constexpr float kExplosionSubLifeTimeMax = 1.8f;
	constexpr float kExplosionSubStartTimeMax = 0.5f;
	constexpr float kExplosionSubRadiusMin = 0.5f;
	constexpr float kExplosionSubRadiusMax = 1.5f;
	constexpr float kExplosionSubScaleMin = 0.15f;
	constexpr float kExplosionSubScaleMax = 0.4f;
	constexpr float kExplosionSubZRange = 0.2f;
	
	// 色定義
	constexpr Vector4 kColorWhite = { 1.0f, 1.0f, 1.0f, 1.0f };
	constexpr Vector4 kColorCyan = { 0.0f, 1.0f, 1.0f, 1.0f };
	constexpr Vector4 kColorGray = { 0.3f, 0.3f, 0.3f, 0.3f };
	constexpr Vector4 kColorBlue = { 0.0f, 0.0f, 1.0f, 1.0f };
	constexpr Vector4 kExplosionColorCenter = { 1.0f, 1.0f, 1.0f, 1.0f };
	constexpr Vector4 kExplosionColorSub = { 1.0f, 0.8f, 0.2f, 1.0f };
	
	// 爆発エフェクトの色遷移
	constexpr float kExplosionColorPhase1 = 0.3f;
	constexpr float kExplosionColorPhase2 = 0.7f;
	constexpr float kExplosionColorPhase3 = 0.9f;
	
	constexpr Vector4 kExplosionColor1 = { 1.0f, 1.0f, 1.0f, 1.0f };
	constexpr Vector4 kExplosionColor2 = { 1.0f, 0.6f, 0.2f, 1.0f };
	constexpr Vector4 kExplosionColor3 = { 1.0f, 1.0f, 0.2f, 1.0f };
	constexpr Vector4 kExplosionColor4 = { 0.5f, 0.5f, 0.5f, 1.0f };
	constexpr Vector4 kExplosionColor5 = { 0.5f, 0.5f, 0.5f, 0.0f };
	
	// 円柱パーティクル
	constexpr Vector3 kCylinderRotation = { -0.2f, 0.0f, 0.0f };
	constexpr float kCylinderLifeTime = 1000.0f;
	
	// UV変換
	constexpr float kCylinderUVStep = 0.0001f;
	
	// ブレンドモード
	enum BlendMode {
		kBlendModeNone = 0,
		kBlendModeNormal = 1,
		kBlendModeAdd = 2,
		kBlendModeSubtract = 3,
		kBlendModeMultiply = 4,
		kBlendModeScreen = 5
	};
	constexpr BlendMode kDefaultBlendMode = kBlendModeAdd;
	
	// 入力レイアウト要素数
	constexpr uint32_t kInputElementCount = 3;
	
	// ルートパラメータ数
	constexpr uint32_t kRootParameterCount = 3;
	
	// 静的サンプラ数
	constexpr uint32_t kStaticSamplerCount = 1;
	
	// シェーダーパス
	constexpr const wchar_t* kVertexShaderPath = L"Resources/shaders/Particle.VS.hlsl";
	constexpr const wchar_t* kPixelShaderPath = L"Resources/shaders/Particle.PS.hlsl";
	constexpr const wchar_t* kVertexShaderProfile = L"vs_6_0";
	constexpr const wchar_t* kPixelShaderProfile = L"ps_6_0";
}

/// <summary>
/// パーティクルを管理するクラス
/// </summary>
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
		bool isExplosion = false;
		bool isSubExplosion = false;
		float maxScale = ParticleManagerConstants::kDefaultParticleScale;
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
		Vector3 acceleration;
		AABB area;
	};

	// 風エリアの構造体
	struct WindZone {
		AABB area;
		Vector3 strength;
	};

	// エミッターの構造体
	struct Emitter {
		Transform transform;
		uint32_t count;
		float frequency;
		float frequencyTime;
		std::string groupName;
	};

	// パーティクルグループの構造体
	struct ParticleGroup {
		MaterialData materialData;
		std::list<Particle> particles;
		uint32_t srvIndex;
		ParticleForGPU* instanceData;
		uint32_t numParticles = 0;
		Microsoft::WRL::ComPtr<ID3D12Resource> instanceBuffer;
	};

	/*------メンバ関数------*/

	// シングルトンインスタンス
	static ParticleManager* GetInstance();

	// コンストラクタ・デストラクタ
	ParticleManager() = default;
	~ParticleManager() = default;
	
	// コピー・ムーブ禁止
	ParticleManager(const ParticleManager&) = delete;
	ParticleManager& operator=(const ParticleManager&) = delete;
	ParticleManager(ParticleManager&&) = delete;
	ParticleManager& operator=(ParticleManager&&) = delete;

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

	// パーティクルの発生
	void Emit(const std::string name, const Vector3& position, uint32_t count);
	void EmitExplosion(const std::string& name, const Vector3& position, uint32_t count);
	void EmitWithVelocity(const std::string& name, const Vector3& position, uint32_t count, const Vector3& velocity);

	// パーティクルの生成
	Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate);
	Particle MakeNewPlaneParticle(std::mt19937& randomEngine, const Vector3& translate);
	Particle MakeNewRingParticle(std::mt19937& randomEngine, const Vector3& translate);
	Particle MakeNewCylinderParticle(std::mt19937& randomEngine, const Vector3& translate);
	Particle MakeNewThrusterParticle(std::mt19937& randomEngine, const Vector3& translate);
	Particle MakeNewSmokeParticle(std::mt19937& randomEngine, const Vector3& translate);

	// パーティクルの更新
	void UpdateExplosionParticle(Particle& particle);

	// 頂点データの作成
	void CreateVertexData();
	void CreateRingVertexData();
	void CreateCylinderVertexData();

	// マテリアルデータの作成
	void CreateMaterialData();

	// ImGuiの描画
	void DrawImGui();

	/*------ゲッター------*/
	bool GetUseBillboard() const { return useBillboard_; }
	bool GetUseRingVertex() const { return useRingVertex_; }
	ParticleType GetParticleType() const { return particleType_; }

	/*------セッター------*/
	void SetUseBillboard(bool useBillboard) { useBillboard_ = useBillboard; }
	void SetUseRingVertex(bool useRingVertex) { useRingVertex_ = useRingVertex; }
	void SetParticleType(ParticleType type);
	void SetParticleScale(const Vector3& scale) { uvTransform_.scale = scale; }
	void SetIsSmoke(bool isSmoke) { isSmoke_ = isSmoke; }

private:
	/*------プライベートメンバ関数------*/
	
	// ルートシグネチャの作成
	void CreateRootSignature();

	// パイプラインステートオブジェクトの作成
	void CreatePSO();

	// AABBとVector3の当たり判定
	bool IsCollision(const AABB& aabb, const Vector3& point);

	// パーティクルグループの更新
	void UpdateParticleGroup(
		ParticleGroup& group,
		const Matrix4x4& billboardMatrix,
		const Matrix4x4& viewProjectionMatrix);

	// パーティクルの更新
	void UpdateParticle(
		Particle& particle,
		ParticleGroup& group,
		const Matrix4x4& billboardMatrix,
		const Matrix4x4& viewProjectionMatrix);

	// ワールド行列の計算
	Matrix4x4 CalculateWorldMatrix(
		const Particle& particle,
		const Matrix4x4& billboardMatrix) const;

	// 風の適用
	void ApplyWind(Particle& particle);

	// パーティクルタイプ別の生成
	Particle CreateParticleByType(const Vector3& position);

	// 頂点バッファの作成
	void CreateVertexBuffer();

	/*------メンバ変数------*/
	
	// DirectX共通部
	DirectXCommon* dxCommon_ = nullptr;

	// SRVマネージャ
	SrvManager* srvManager_ = nullptr;

	// カメラ
	Camera* camera_ = nullptr;

	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

	// グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

	// リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;

	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	// モデルデータ
	ModelData modelData_;

	// 頂点データ
	VertexData* vertexData_ = nullptr;

	// マテリアルデータ
	Material* materialData_ = nullptr;

	// パーティクルタイプ
	ParticleType particleType_ = ParticleType::Normal;

	// ビルボードフラグ
	bool useBillboard_ = false;

	// リング型頂点フラグ
	bool useRingVertex_ = false;

	// 風フラグ
	bool isWind_ = false;

	// 煙フラグ
	bool isSmoke_ = false;

	// UV変換
	Transform uvTransform_{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};

	// 加速度フィールド
	AccelerationField accelerationField_;

	// 風エリア（データとして外部化推奨）
	std::vector<WindZone> windZones_{

		{ { {-5.0f, -5.0f, -5.0f}, {5.0f, 5.0f, 5.0f} }, {0.1f, 0.0f, 0.0f} },
		{ { {10.0f, -5.0f, -5.0f}, {15.0f, 5.0f, 5.0f} }, {5.0f, 0.0f, 0.0f} }
	};

	// ランダムエンジン
	std::random_device seedGeneral_;
	std::mt19937 randomEngine_;

	// パーティクルグループ
	std::unordered_map<std::string, ParticleGroup> particleGroups_;
};


#pragma once
#include <Object3d.h>
#include <Input.h>
#include <Camera.h>
#include <Collider.h>
#include <WorldTransform.h>
#include <memory>
#include <cstdint>

// 前方宣言
struct Vector3;

/// <summary>
/// キャラクターの基底クラス
/// - ゲーム内の移動・攻撃可能なエンティティの共通インターフェースを提供
/// - Colliderを継承し、衝突判定機能を持つ
/// - Input, Camera, WorldTransformに依存し、3D空間での振る舞いを実現
/// - 派生クラスでMove()とAttack()の実装が必須
/// </summary>
class BaseCharacter : public Collider
{
public:
	/// <summary>
	/// 初期化処理
	/// - Input, Camera, WorldTransformのインスタンスを取得・初期化
	/// - 派生クラスで追加の初期化が必要な場合はオーバーライド
	/// </summary>
	virtual void Initialize();

	/// <summary>
	/// 更新処理
	/// - ワールド変換をObject3dに反映
	/// - 派生クラスで移動・攻撃等の処理を実装
	/// </summary>
	virtual void Update();

	/// <summary>
	/// 描画処理
	/// - 3Dオブジェクトを描画
	/// - 派生クラスで追加の描画処理が必要な場合はオーバーライド
	/// </summary>
	virtual void Draw();

	/// <summary>
	/// キャラクターの移動処理（純粋仮想関数）
	/// - 派生クラスで具体的な移動ロジックを実装
	/// </summary>
	virtual void Move() = 0;

	/// <summary>
	/// キャラクターの攻撃処理（純粋仮想関数）
	/// - 派生クラスで具体的な攻撃ロジックを実装
	/// </summary>
	virtual void Attack() = 0;

	/// <summary>
	/// 衝突判定時のコールバック
	/// - Colliderから継承
	/// </summary>
	/// <param name="other">衝突した相手のColliderポインタ</param>
	virtual void OnCollision(Collider* other) override;

	/// <summary>
	/// 中心座標を取得（純粋仮想関数のオーバーライド）
	/// - Colliderから継承
	/// - 派生クラスで実際の中心座標を返すよう実装
	/// </summary>
	/// <returns>キャラクターの中心座標</returns>
	virtual Vector3 GetCenterPosition() const override;

	/*------ゲッター------*/

	/// <summary>
	/// ヒットポイントを取得
	/// </summary>
	/// <returns>現在のHP</returns>
	int GetHp() const { return hp_; }

	/// <summary>
	/// ワールド変換を取得
	/// </summary>
	/// <returns>WorldTransformの参照</returns>
	WorldTransform& GetWorldTransform() { return worldTransform_; }

	/// <summary>
	/// スケールを取得
	/// </summary>
	/// <returns>現在のスケール値</returns>
	const Vector3& GetScale() const { return worldTransform_.GetScale(); }

	/// <summary>
	/// 回転を取得
	/// </summary>
	/// <returns>現在の回転値</returns>
	const Vector3& GetRotation() const { return worldTransform_.GetRotate(); }

	/// <summary>
	/// 座標を取得
	/// </summary>
	/// <returns>現在の座標</returns>
	const Vector3& GetPosition() const { return worldTransform_.GetTranslate(); }

	/// <summary>
	/// シリアルナンバーを取得
	/// </summary>
	/// <returns>このキャラクター固有のシリアルナンバー</returns>
	uint32_t GetSerialNumber() const { return serialNumber_; }
	
	/// <summary>
	/// 生存状態を取得
	/// </summary>
	/// <returns>生存している場合true</returns>
	bool IsAlive() const { return isAlive_; }

	/*------セッター------*/

	/// <summary>
	/// ヒットポイントを設定
	/// </summary>
	/// <param name="hp">設定するHP値</param>
	void SetHp(int hp) { hp_ = hp; }

	/// <summary>
	/// スケールを設定
	/// </summary>
	/// <param name="scale">設定するスケール値</param>
	void SetScale(const Vector3& scale) { worldTransform_.SetScale(scale); }

	/// <summary>
	/// 回転を設定
	/// </summary>
	/// <param name="rotation">設定する回転値</param>
	void SetRotation(const Vector3& rotation) { worldTransform_.SetRotate(rotation); }

	/// <summary>
	/// 座標を設定
	/// </summary>
	/// <param name="position">設定する座標</param>
	void SetPosition(const Vector3& position) { worldTransform_.SetTranslate(position); }

	/// <summary>
	/// シリアルナンバーを設定
	/// </summary>
	/// <param name="serialNumber">設定するシリアルナンバー</param>
	void SetSerialNumber(uint32_t serialNumber) { serialNumber_ = serialNumber; }

protected:
	/*------メンバ変数------*/

	// 3Dオブジェクト（描画用）
	std::unique_ptr<Object3d> object3d_;

	// ワールド変換（位置・回転・スケール管理）
	WorldTransform worldTransform_;

	// 入力（Input::GetInstance()から取得）
	Input* input_ = nullptr;

	// カメラ（Object3dCommon経由で取得）
	Camera* camera_ = nullptr;

	// ヒットポイント
	uint32_t hp_ = 10;

	// 生存フラグ（falseで死亡扱い）
	bool isAlive_ = true;

	// シリアルナンバー（キャラクター識別用）
	uint32_t serialNumber_ = 0;

	// 次のシリアルナンバー（全キャラクター共通のカウンタ）
	static inline uint32_t sNextSerialNumber_ = 0;
};


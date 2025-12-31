#pragma once
#include <Object3d.h>
#include <Input.h>
#include <Camera.h>
#include <Collider.h>
#include <WorldTransform.h>
/// <sumary>
/// キャラクターの基底クラス
/// <sumary>
class BaseCharacter : public Collider
{
public:
	// 初期化
	virtual void Initialize();

	// 更新
	virtual void Update();

	// 描画
	virtual void Draw();

	// キャラクターの移動
	virtual void Move() = 0;

	// キャラクターの攻撃
	virtual void Attack() = 0;

	// 衝突判定
	virtual void OnCollision(Collider* other) override;

	// 中心座標を取得する純粋仮想関数
	virtual Vector3 GetCenterPosition() const override;

	/*------ゲッター------*/

	// ヒットポイントを取得
	int GetHp() const { return hp_; }

	WorldTransform& GetWorldTransform() { return worldTransform_; }

	// スケールを取得
	const Vector3& GetScale() const { return worldTransform_.GetScale(); }

	// 回転を取得
	const Vector3& GetRotation() const { return worldTransform_.GetRotate(); }

	// 座標を取得
	const Vector3& GetPosition() const { return worldTransform_.GetTranslate(); }

	// シリアルナンバーを取得
	uint32_t GetSerialNumber() const { return serialNumber_; }
	
	// 生存状態を取得
	bool IsAlive() const { return isAlive_; }

	/*------セッター------*/
	// ヒットポイントを設定
	void SetHp(int hp) { hp_ = hp; }

	// スケールを設定
	void SetScale(const Vector3& scale) { worldTransform_.SetScale(scale); }

	// 回転を設定
	void SetRotation(const Vector3& rotation) { worldTransform_.SetRotate(rotation); }

	// 座標を設定
	void SetPosition(const Vector3& position) { worldTransform_.SetTranslate(position); }

	// シリアルナンバーを設定
	void SetSerialNumber(uint32_t serialNumber) { serialNumber_ = serialNumber; }

protected:
	/*------メンバ変数------*/
	std::unique_ptr<Object3d> object3d_; // 3Dオブジェクト

	// ワールド変換
	WorldTransform worldTransform_;

	// 入力
	Input* input_ = nullptr;

	// カメラ
	Camera* camera_ = nullptr;

	// ヒットポイント
	uint32_t hp_ = 10;

	// 生存フラグ
	bool isAlive_ = true;

	// シリアルナンバー
	uint32_t serialNumber_ = 0;

	// 次のシリアルナンバー
	static inline uint32_t sNextSerialNumber_ = 0;
};


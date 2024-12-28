#pragma once
#include <DirectXCommon.h>
#include <Input.h>
#include <ModelManager.h>
#include <Object3dCommon.h>
#include <SpriteCommon.h>
#include <Object3d.h>
#include <Sprite.h>
#include <DierctXGame/application/object/Player.h>
#include <Enemy.h>
#include <EnemyBullet.h>


class GameScene
{
public:
	~GameScene();
	/// 初期化
	void Initialize();

	/// 更新
	void Update();

	/// 描画
	void Draw();

	/// <summary>
	/// 衝突判定と応答
	/// </summary>
	void CheckAllCollisions();

	/// <summary>
	/// 敵弾を追加する
	/// </summary>
	void AddEnemyBullet(EnemyBullet* enemyBullet);

	// 弾リストを取得
	const std::list<EnemyBullet*>& GetEnemyBullets() const { return enemyBullets_; }


	/// <summary>
	/// 敵発生データの読み込み
	/// </summary>
	void LoadEnemyPopData();

	/// <summary>
	/// 敵発生コマンドの更新
	/// </summary>
	void UpdateEnemyPopCommands();

	/// <summary>
	/// 敵発生
	/// </summary>
	void enemyPop(Vector3 translation);
private:
	// 基盤
	DirectXCommon* dxCommon_;

	Input* input_;

	Object3dCommon* object3dCommon_;

	SpriteCommon* spriteCommon_;

	Camera* camera_;

	std::vector<Sprite*> sprites;

	std::vector<Object3d*> object3ds;

	// ゲームに関する変数

	Transform cameraTransform{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,-10.0f}
	};

	Player* player_;

	Object3d* playerModel;

	// 弾丸のモデルを生成
	Object3d* bulletModel;

	// 敵
	Enemy* enemy_ = nullptr;

	Object3d* enemyModel;

	Vector3 enemyPosition_ = { 5.0f, 3.0f, 50.0f };

	// 敵リスト
	std::list<Enemy*> enemies_;

	// 敵弾リストの取得
	std::list<EnemyBullet*> enemyBullets_;


	// 敵発生
	std::stringstream enemyPopCommands;

	// 待機中フラグ
	bool isWait = false;

	// 待機タイマー
	int32_t kWaitTimer_ = 0;

	float playerRadius_ = 1.0f;

	float playerBulletRadius_ = 1.0f;

	float enemyRadius_ = 1.0f;

	float enemyBulletRadius_ = 1.0f;
};

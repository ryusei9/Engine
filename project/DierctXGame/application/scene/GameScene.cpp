#include "GameScene.h"

GameScene::~GameScene()
{
	delete object3dCommon_;
	delete spriteCommon_;
	delete camera_;
	for (auto& object3d : object3ds) {
		delete object3d;
	}
	for (auto& sprite : sprites) {
		delete sprite;
	}
}

void GameScene::Initialize()
{
	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();

	object3dCommon_ = new Object3dCommon;
	object3dCommon_->Initialize(dxCommon_);

	spriteCommon_ = new SpriteCommon;
	spriteCommon_->Initialize(dxCommon_);

	// テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(dxCommon_);

	// 3Dモデルマネージャの初期化
	ModelManager::GetInstance()->Initialize(dxCommon_);

	camera_ = new Camera;
	camera_->SetRotate({ 0.0f,0.0f,0.0f });
	camera_->SetTranslate({ 0.0f,0.0f,-10.0f });
	object3dCommon_->SetDefaultCamera(camera_);

	for (uint32_t i = 0; i < 5; ++i) {
		if (i < 4) {
			Sprite* sprite = new Sprite();
			sprite->Initialize(spriteCommon_, dxCommon_, "resources/uvChecker.png");
			sprites.push_back(sprite);
		}
		else {
			Sprite* sprite = new Sprite();
			sprite->Initialize(spriteCommon_, dxCommon_, "resources/monsterBall.png");
			sprites.push_back(sprite);
		}
	}

	//std::vector<Model*> models;
	for (uint32_t i = 0; i < 2; ++i) {
		Object3d* object3d = new Object3d();
		object3d->Initialize(object3dCommon_);
		object3ds.push_back(object3d);
	}
	// 初期化済みの3Dオブジェクトにモデルを紐づける
	object3ds[0]->SetModel("plane.obj");
	object3ds[1]->SetModel("axis.obj");
}

void GameScene::Update()
{
	// 入力の更新
	input_->Update();
	// 操作
	if (input_->PushKey(DIK_RIGHTARROW)) {
		cameraTransform.translate.x -= 0.1f;

	}
	if (input_->PushKey(DIK_LEFTARROW)) {
		cameraTransform.translate.x += 0.1f;
	}
	camera_->SetTranslate(cameraTransform.translate);
	camera_->Update();

	for (uint32_t i = 0; i < sprites.size();++i) {

		sprites[i]->Update();

		// 現在の座標を変数で受ける
		Vector2 position = sprites[i]->GetPosition();

		// 座標を変更する
		
		// 変更を反映する
		sprites[i]->SetPosition({ 200.0f * i });

		// 角度を変更させるテスト
		float rotation = sprites[i]->GetRotation();

		

		sprites[i]->SetRotation(rotation);

		// 色を変化させるテスト
		Vector4 color = sprites[i]->GetColor();

		

		sprites[i]->SetColor(color);

		// サイズを変化させるテスト
		Vector2 size = sprites[i]->GetSize();
		

		sprites[i]->SetSize({ 100.0f,100.0f });

		// 反転X
		bool isFlipX = sprites[i]->GetIsFlipX();
		

		sprites[i]->SetIsFlipX(isFlipX);

		bool isFlipY = sprites[i]->GetIsFlipY();
		

		sprites[i]->SetIsFlipY(isFlipY);
	}

	for (uint32_t i = 0; i < object3ds.size();++i) {
		object3ds[i]->Update();
		// 現在の座標を変数で受ける
		Vector3 position[2];
		position[i] = object3ds[i]->GetTranslate();
		// 座標を変更する
		
		position[0].x = -2.0f;
		position[1].x = 2.0f;
		object3ds[i]->SetTranslate(position[i]);

		// 角度を変更させるテスト
		Vector3 rotation[2];
		rotation[i] = object3ds[i]->GetRotate();

		
		rotation[0].y += 0.01f;
		rotation[1].z += 0.01f;
		object3ds[i]->SetRotate(rotation[i]);

		// 拡縮を変更するテスト
		Vector3 scale[2];
		scale[i] = object3ds[i]->GetScale();
		
		object3ds[i]->SetScale(scale[i]);
	}

}

void GameScene::Draw()
{
	// 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックコマンドを積む
	object3dCommon_->DrawSettings();
	// 全てのobject3d個々の描画
	for (auto& object3d : object3ds) {
		object3d->Draw();
	}
	// Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
	spriteCommon_->DrawSettings();

	for (auto& sprite : sprites) {
		sprite->Draw();
	}
}

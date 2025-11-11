#include "Cube.h"

Cube::Cube(const std::string& texturePath)
    : texturePath_(texturePath) {}

void Cube::Initialize() {
    // ddsファイルを読み込む
    TextureManager::GetInstance()->LoadTexture(texturePath_);
    // SRVインデックスを取得
    textureSrvIndex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath(texturePath_);
    // 頂点バッファ・インデックスバッファ等の初期化もここで行う
}

void Cube::Draw() {
    auto objCommon = Object3dCommon::GetInstance();
    auto srvManager = objCommon->GetSrvManager();
    // SRVをバインド
    srvManager->SetGraphicsRootDescriptorTable(2, textureSrvIndex_);
    // キューブの描画コマンド発行
    // ...（頂点バッファ・インデックスバッファのセット等）
}
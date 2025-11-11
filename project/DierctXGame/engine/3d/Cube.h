#pragma once
#include <string>
#include <memory>
#include "Object3dCommon.h"
#include "TextureManager.h"

class Cube {
public:
    Cube(const std::string& texturePath);

    void Initialize();
    void Draw();

private:
    std::string texturePath_;
    uint32_t textureSrvIndex_ = 0;
    // 頂点バッファやインデックスバッファ等もここで管理
};
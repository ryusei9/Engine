#include "Skybox.hlsli"

struct TransformationMatrix{
    float32_t4x4 WVP;
    float32_t4x4 World;
    float32_t4x4 WorldInverseTranspose;
};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct VertexShaderInput{
    float32_t4 position : POSITION0;
    float32_t3 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};

struct Material{
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
};

struct DirectionalLight
{
    float32_t4 color; // ライトの色
    float32_t3 direction; // ライトの向き
    float intensity; // 輝度
};

VertexShaderOutput main(VertexShaderInput input){
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformationMatrix.WVP).xyww;
    output.texcoord = input.position.xyz;
    //output.normal = normalize(mul(input.normal, (float32_t3x3) gTransformationMatrix.WorldInverseTranspose));
    //output.worldPosition = mul(input.position, gTransformationMatrix.World).xyz;
    return output;
}
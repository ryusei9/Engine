#include "Object3d.hlsli"

struct Material{
    float32_t4 color;
    uint32_t enableLighting;
    float32_t4x4 uvTransform;
};

struct DirectionalLight
{
    float32_t4 color; // ライトの色
    float32_t3 direction; // ライトの向き
    float intensity; // 輝度
};

struct TransformationMatrix
{
    float32_t4x4 WVP;
    float32_t4x4 World;
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
struct PixelShaderOutput {
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input){
    PixelShaderOutput output;
    // 不透明な白を出力する
    output.color = gMaterial.color;
    //float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    //output.color = gMaterial.color * textureColor;
    float4 transformedUV = mul(float32_t4(input.texcoord,0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    if (gMaterial.enableLighting != 0)
    { // Lightingする場合
        // half lambert
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
    }
    else
    { // Lightingしない場合。前回までと同じ演算
        output.color = gMaterial.color * textureColor;
    }
    return output;
}


#include "FullScreen.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
Texture2D<float32_t> gMaskTexture : register(t1);
cbuffer DissolveParams : register(b1)
{
    float threshold;
    float edgeWidth; // エッジの距離（幅）
    float edgeStrength; // エッジの強さ
    float3 edgeColor; // エッジの色
};

struct PixelShaderOutput
{
	float32_t4 color : SV_TARGET;
};
PixelShaderOutput main(VertexShaderOutput input)
{
    float32_t mask = gMaskTexture.Sample(gSampler, input.texcoord);
	// maskの値がthreshold(閾値)以下の場合はdiscardして抜く
    if (mask <= threshold)
    {
		discard;
    }
	// エッジの幅を可変に
    float32_t edge = 1.0f - smoothstep(threshold, threshold + edgeWidth, mask);
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, input.texcoord);
    // エッジ色・強さを可変に
    output.color.rgb += edge * edgeStrength * edgeColor;
    return output;
}
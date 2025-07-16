#include "FullScreen.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
cbuffer gMaterial : register(b0)
{
    float time;
};

struct PixelShaderOutput
{
	float32_t4 color : SV_TARGET;
};
// 2D座標から0～1の乱数を生成する関数
float rand2dTo1d(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

PixelShaderOutput main(VertexShaderOutput input)
{
	PixelShaderOutput output;
	// 乱数生成。引数にtexcoordとtimeを使う
    float32_t random = rand2dTo1d(input.texcoord * time);
    // 入力画像の色を取得
    float32_t4 texColor = gTexture.Sample(gSampler, input.texcoord);
    // 画像色に乱数を乗算して出力
    output.color = texColor * random;
    output.color.a = texColor.a; // アルファは元画像のままにしたい場合
    return output;
}
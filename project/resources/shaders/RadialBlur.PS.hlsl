#include "FullScreen.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
	float32_t4 color : SV_TARGET;
};
PixelShaderOutput main(VertexShaderOutput input)
{
    const float32_t2 kCenter = float32_t2(0.5f, 0.5f);	// 中心点。ここを基準に放射状にブラーがかかる
    const int32_t kNumSamples = 10; // サンプル数。多いほどブラーが強くなる
    const float32_t kBlurWidth = 0.01f; // ブラーの幅。大きいほどブラーが強くなる
	// 中心から現在のuvに対しての方向を計算
    float32_t2 direction = input.texcoord - kCenter;
    float32_t3 outputColor = float32_t3(0.0f, 0.0f, 0.0f);
    for (int32_t sampleIndex = 0; sampleIndex < kNumSamples; ++sampleIndex)
    {
        // 現在のuvから先ほど計算した方向にサンプリング店を進めながらサンプリングしていく
        float32_t2 texcoord = input.texcoord + direction * kBlurWidth * float32_t(sampleIndex);
        outputColor.rgb += gTexture.Sample(gSampler, texcoord).rgb;
    }
    // 平均化する
    outputColor.rgb *= rcp(kNumSamples);
    PixelShaderOutput output;
    output.color.rgb = outputColor;
    output.color.a = 1.0f; // アルファ値は1.0に設定
	return output;
}
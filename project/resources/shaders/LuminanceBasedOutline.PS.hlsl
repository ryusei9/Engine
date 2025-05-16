#include "FullScreen.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
	float32_t4 color : SV_TARGET;
};

static const float32_t2 kIndex3x3[3][3] =
{
    { { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f } },
    { { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { -1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } },
};

static const float32_t kPrewittHorizontalKernel[3][3] =
{
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
};

static const float32_t kPrewittVerticalKernel[3][3] =
{
    { -1.0f / 6.0f, -1.0f / 6.0f, -1.0f / 6.0f },
    { 0.0f, 0.0f, 0.0f },
    { 1.0f / 6.0f, 1.0f / 6.0f, 1.0f / 6.0f },
};

float32_t Luminance(float32_t3 v)
{
    return dot(v, float32_t3(0.2125f, 0.7154f, 0.0721f));
}

PixelShaderOutput main(VertexShaderOutput input)
{
    // uvstepSizeの算出
    uint32_t width, height;
    gTexture.GetDimensions(width, height);
    float32_t2 uvStepSize = float32_t2(rcp(width), rcp(height));
   
    
    // 縦横それぞれの畳み込みの結果を格納する変数
    float32_t2 differrence = float32_t2(0.0f, 0.0f);
    // 色を輝度に変換して、畳み込みを行っていく
    for (int32_t x = 0; x < 3; ++x)
    {
        for (int32_t y = 0; y < 3; ++y)
        {
            // 現在のtexcoordを算出
            float32_t2 texcoord = input.texcoord + kIndex3x3[x][y] * uvStepSize;
            // 色に1/9を掛けて加算
            float32_t3 fetchColor = gTexture.Sample(gSampler, texcoord).rgb;
            float32_t luminance = Luminance(fetchColor);
            differrence.x += luminance * kPrewittHorizontalKernel[x][y];
            differrence.y += luminance * kPrewittVerticalKernel[x][y];
        }
    }
    // 変化の長さをウェイトとして合成
    float32_t weight = length(differrence * 6.0f);
    weight = saturate(weight);
     
    PixelShaderOutput output;
    output.color.rgb = (1.0f - weight) * gTexture.Sample(gSampler, input.texcoord).rgb;
    output.color.a = 1.0f;
    
	return output;
}
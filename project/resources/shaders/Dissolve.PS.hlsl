#include "FullScreen.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
Texture2D<float32_t> gMaskTexture : register(t1);

struct PixelShaderOutput
{
	float32_t4 color : SV_TARGET;
};
PixelShaderOutput main(VertexShaderOutput input)
{
    float32_t mask = gMaskTexture.Sample(gSampler, input.texcoord);
	// maskの値が0.5(閾値)以下の場合はdiscardして抜く
	if(mask <= 0.5f) {
		discard;
    }
	// edgeっぽさを算出
    float32_t edge = 1.0f - smoothstep(0.5f, 0.53f, mask);
	PixelShaderOutput output;
	output.color = gTexture.Sample(gSampler, input.texcoord);
	// edgeっぽいほどしていした色を加算
    output.color.rgb += edge * float32_t3(1.0f, 0.4f, 0.3f);
	return output;
}
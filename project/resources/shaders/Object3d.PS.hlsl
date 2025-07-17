#include "Object3d.hlsli"

struct Material{
    float32_t4 color;
    uint32_t enableLighting;
    float32_t4x4 uvTransform;
    float32_t shininess;
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

struct Camera
{
    float32_t3 worldPosition;
};

struct PointLight
{
    float32_t4 color; // ライトの色
    float32_t3 position; // ライトの位置
    float intensity; // 輝度
    float radius; // ライトの届く最大距離
    float decay; // 減衰率
};

struct SpotLight
{
    float32_t4 color; // ライトの色
    float32_t3 position; // ライトの位置
    float32_t intensity; // 輝度
    float32_t3 direction; // ライトの向き
    float32_t distance; // ライトの届く最大距離
    float32_t decay; // 減衰率
    float32_t cosAngle; // スポットライトの角度
    float32_t cosFalloffStart; // 開始角度
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
TextureCube<float32_t4> gEnvironmentTexture : register(t1); // 環境マッピング用のキューブマップ

ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);
ConstantBuffer<PointLight> gPointLight : register(b3);
ConstantBuffer<SpotLight> gSpotLight : register(b4);
struct PixelShaderOutput {
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input){
    PixelShaderOutput output;

    // UV設定
    float32_t4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy); // テクスチャの色
    textureColor.rgb = pow(textureColor.rgb, 2.2f); // ガンマ補正済みのテクスチャの場合、リニア空間に変換
    
     // 照明効果の統合
    if (gMaterial.enableLighting != 0)
    {
        // ライト方向と法線、カメラ方向の計算
        float32_t3 lightDir = normalize(gDirectionalLight.direction); // ライト方向（逆方向）  
        float32_t3 normal = normalize(input.normal); // 法線の正規化
        float32_t3 viewDir = normalize(gCamera.worldPosition - input.worldPosition); // 視線方向（カメラ方向）

        // 環境光（Ambient）
        float32_t3 ambientColor = gMaterial.color.rgb * gDirectionalLight.color.rgb * gDirectionalLight.intensity * 0.1f; // 環境光を少し抑える
    
        /*------平行光源------*/
    
        // ハーフランバート反射の計算
        float NdotL = dot(normal, lightDir); // 法線と光の角度
        float halfLambertFactor = saturate(pow(NdotL * 0.5f + 0.5f, 2.0f)); // ハーフランバート反射
    
        // 平行光源の拡散反射（Diffuse）
        float32_t3 diffuseColor = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * gDirectionalLight.intensity * halfLambertFactor;

        // 平行光源の鏡面反射（Specular）
        float32_t3 specularColor = float32_t3(0.0f, 0.0f, 0.0f);
        if (gDirectionalLight.intensity > 0.0f && NdotL > 0.0f)
        {
            float32_t3 halfVector = normalize(lightDir + viewDir);
            float NdotH = max(dot(normal, halfVector), 0.0f);
            float shininess = max(gMaterial.shininess, 50.0f); // 光沢度を調整
            specularColor = float32_t3(1.0f, 1.0f, 1.0f) * pow(NdotH, shininess) * gDirectionalLight.intensity;
        }
    
        /*------ポイントライト------*/
   
        // ポイントライトの方向
        float32_t3 pointLightDir = gPointLight.position - input.worldPosition;
        float distance = length(pointLightDir); // ポイントライトへの距離
        pointLightDir = normalize(pointLightDir); // 正規化
    
        // 減衰の計算（逆二乗の法則）
        float attenuation = 1.0f / (1.0f + gPointLight.decay * pow(distance / (gPointLight.radius + 1.0f), gPointLight.decay));
        attenuation = saturate(attenuation); // 0～1にクランプ
    
        // ポイントライトのハーフランバート反射の計算
        float pointNdotL = dot(normal, pointLightDir);
        float pointLightHalfLambertFactor = saturate(pow(pointNdotL * 0.5f + 0.5f, 2.0f)); // ハーフランバート反射
    
        // ポイントライトの拡散反射
        float32_t3 pointDiffuseColor = gMaterial.color.rgb * textureColor.rgb * gPointLight.color.rgb * gPointLight.intensity * pointLightHalfLambertFactor * attenuation;
    
        // ポイントライトの鏡面反射
        float32_t3 pointSpecularColor = float32_t3(0.0f, 0.0f, 0.0f);
        if (gPointLight.intensity > 0.0f && pointNdotL > 0.0f)
        {
            float32_t3 pointHalfVector = normalize(pointLightDir + viewDir);
            float pointNdotH = max(dot(normal, pointHalfVector), 0.0f);
            float shininess = max(gMaterial.shininess, 50.0f);
            pointSpecularColor = float32_t3(1.0f, 1.0f, 1.0f) * pow(pointNdotH, shininess) * gPointLight.intensity;
        }
    
        /*------スポットライト------*/
    
        float32_t3 spotLightDir = gSpotLight.position - input.worldPosition; // スポットライトからピクセルへの方向
        float spotLightDistance = length(spotLightDir); // 距離
        spotLightDir = normalize(spotLightDir); // 正規化
    
        // 距離減衰と逆二乗の法則を計算
        float spotAttenuation = 1.0f / (1.0f + gSpotLight.decay * pow(spotLightDistance / (gSpotLight.distance + 1.0f), 2.0f));
        spotAttenuation = saturate(spotAttenuation); // クランプ
    
        // 角度減衰の計算
        float cosAngle = dot(-spotLightDir, normalize(gSpotLight.direction));
        float spotAngleFactor = saturate((cosAngle - gSpotLight.cosAngle) / (gSpotLight.cosFalloffStart - gSpotLight.cosAngle)); // 緩やかに減衰
    
        // スポットライトの拡散反射
        float spotNdotL = dot(normal, spotLightDir); // 法線とライト方向
        float spotLghtHalfLambertFactor = saturate(pow(spotNdotL * 0.5f + 0.5f, 2.0f)); // ハーフランバート反射
        float32_t3 spotDiffuseColor = gMaterial.color.rgb * textureColor.rgb * gSpotLight.color.rgb * gSpotLight.intensity * spotLghtHalfLambertFactor * spotAttenuation * spotAngleFactor;
    
        // スポットライトの鏡面反射
        float32_t3 spotSpecularColor = float32_t3(0.0f, 0.0f, 0.0f);
        if (gSpotLight.intensity > 0.0f && spotNdotL > 0.0f)
        {
            float32_t3 spotHalfVector = normalize(spotLightDir + viewDir); // ハーフベクトル
            float spotNdotH = max(dot(normal, spotHalfVector), 0.0f);
            float shininess = max(gMaterial.shininess, 50.0f);
            spotSpecularColor = float32_t3(1.0f, 1.0f, 1.0f) * pow(spotNdotH, shininess) * gSpotLight.intensity * spotAttenuation * spotAngleFactor;
        }
        
        // 環境光 + 拡散反射 + 鏡面反射 + 点光源の拡散反射 + 点光源の鏡面反射 + スポットライトの拡散反射 + スポットライトの鏡面反射
        float32_t3 finalColor = /*ambientColor + */diffuseColor + specularColor + pointDiffuseColor + pointSpecularColor + spotDiffuseColor + spotSpecularColor;
        output.color.rgb = saturate(finalColor);
        
        // 環境マッピングの計算
        float32_t3 cameraToPosition = normalize(input.worldPosition - gCamera.worldPosition);
        float32_t3 reflectedVector = reflect(cameraToPosition, normalize(input.normal));
        float32_t4 environmentColor = gEnvironmentTexture.Sample(gSampler, reflectedVector);
        output.color.rgb += environmentColor.rgb;
        

        // ガンマ補正を適用（必要なら）
        //output.color.rgb = pow(output.color.rgb, 1.0f / 2.2f);
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    {
        output.color = gMaterial.color * textureColor;

        // ガンマ補正は不要（出力次第で適用）
        output.color.rgb = pow(output.color.rgb, 1.0f / 2.2f);
    }

    // α値がほぼ0の場合にピクセルを破棄
    if (output.color.a < 0.001f)
    {
        discard;
    }

    return output;
}
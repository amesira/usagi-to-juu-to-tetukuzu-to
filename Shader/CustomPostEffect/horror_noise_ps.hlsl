// horror_noise_ps.hlsl
Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

// 入力用構造体
struct PS_INPUT
{
    float4 posH : SV_Position;
    float2 texcoord   : TEXCOORD;
};

cbuffer HorrorNoiseBuffer : register(b0) {
    float noiseMin;
    float noiseMax;
    float contrastPow;
};

float Rand(float2 coord){
    return frac(sin(dot(coord.xy, float2(12.9898, 78.233))) * 43758.5453);
}

float4 main(PS_INPUT ps_in) : SV_TARGET
{
    float4 tex = g_Texture.Sample(g_SamplerState, ps_in.texcoord);
    
    // ノイズの生成・輝度調整
    float noise = Rand(ps_in.texcoord * 1000.0f);
    tex.rgb += noise * (noiseMax - noiseMin) + noiseMin;
    
    float4 color;
    color.rgb = saturate(tex.rgb);
    color.a = tex.a;
    
    // グレースケール
    color.rgb = dot(color.rgb, float3(0.299f, 0.587f, 0.114f));
    // コントラスト調整
    color.rgb = pow(color.rgb, contrastPow);
    
    return color;
}

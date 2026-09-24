// posterize_ps.hlsl
Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

// 入力用構造体
struct PS_INPUT
{
    float4 posH : SV_Position;
    float2 texcoord   : TEXCOORD;
};

cbuffer PosterizeBuffer : register(b0) {
    int g_Levels; // ポスタリゼーションのレベル数
    float g_Strength;
    float2 g_Padding;
};

float4 main(PS_INPUT ps_in) : SV_TARGET
{
    float4 color = g_Texture.Sample(g_SamplerState, ps_in.texcoord);
    
    int safeLevels = max(g_Levels, 2);
    float div = 1.0f / (safeLevels - 1);
    float3 posterized = floor(color.rgb / div + 0.5f) * div;
    color.rgb = lerp(color.rgb, saturate(posterized), saturate(g_Strength));
    
    // MEMO 各諧調の中間をとるための+0.5f
    
    return color;
}

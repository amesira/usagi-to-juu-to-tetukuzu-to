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
    int levels; // ポスタリゼーションのレベル数
};

float4 main(PS_INPUT ps_in) : SV_TARGET
{
    float4 color = g_Texture.Sample(g_SamplerState, ps_in.texcoord);
    
    // ポスタライズ処理
    float div = 1.0f / (levels - 1);
    color.rgb = floor(color.rgb / div + 0.5f) * div;
    color.rgb = saturate(color.rgb);
    
    // MEMO 各諧調の中間をとるための+0.5f
    
    return color;
}

// chromatic_aberration_ps.hlsl
Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

// 入力用構造体
struct PS_INPUT
{
    float4 posH : SV_Position;
    float2 texcoord   : TEXCOORD;
};

cbuffer ChromaticAberrationBuffer : register(b0) {
    float rShift;
    float bShift;
};

float4 main(PS_INPUT ps_in) : SV_TARGET
{
    float4 color;
    
    float2 uv = ps_in.texcoord;
    
    color.r = g_Texture.Sample(g_SamplerState, uv + float2(rShift, 0.0f)).r;
    color.g = g_Texture.Sample(g_SamplerState, uv).g;
    color.b = g_Texture.Sample(g_SamplerState, uv + float2(bShift, 0.0f)).b;
    
    color.a = g_Texture.Sample(g_SamplerState, uv).a;
    
    return color;
}
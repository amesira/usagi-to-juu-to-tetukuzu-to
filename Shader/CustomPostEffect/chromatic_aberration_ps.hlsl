// chromatic_aberration_ps.hlsl
Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

struct PS_INPUT
{
    float4 posH : SV_Position;
    float2 texcoord   : TEXCOORD;
};

cbuffer ChromaticAberrationBuffer : register(b0) {
    float2 g_InvScreenSize;
    float g_RShiftPixels;
    float g_BShiftPixels;
    float g_Strength;
    
    float3 padding;
};

float4 main(PS_INPUT ps_in) : SV_TARGET
{
    float2 uv = ps_in.texcoord;
    float2 rOffset = float2(g_RShiftPixels * g_Strength, 0.0f) * g_InvScreenSize;
    float2 bOffset = float2(g_BShiftPixels * g_Strength, 0.0f) * g_InvScreenSize;

    float4 color;
    color.r = g_Texture.Sample(g_SamplerState, uv + rOffset).r;
    color.g = g_Texture.Sample(g_SamplerState, uv).g;
    color.b = g_Texture.Sample(g_SamplerState, uv + bOffset).b;
    color.a = g_Texture.Sample(g_SamplerState, uv).a;
    return color;
}

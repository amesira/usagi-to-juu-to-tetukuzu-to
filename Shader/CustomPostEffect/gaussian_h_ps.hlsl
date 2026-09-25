// gaussian_h_ps.hlsl
// 縦向きブラー用PS
Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

struct PS_INPUT
{
    float4 posH : SV_Position;
    float2 texcoord : TEXCOORD;
};

cbuffer GaussianBuffer : register(b0) {
    float g_Strength; // ブラーの強さ
};

float4 main(PS_INPUT ps_in) : SV_TARGET
{
    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    return color;
}

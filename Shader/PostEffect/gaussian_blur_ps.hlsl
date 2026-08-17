//+++++++++++++++++++++++++++++++++++++++++++++++++++
// gaussian_blur_ps.hlsl
// ・ガウシアンブラーのピクセルシェーダー
// 
// Author：Miu Kitamura
// Date  ：2026/05/06
//+++++++++++++++++++++++++++++++++++++++++++++++++++
Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

// 入力用構造体
struct PS_INPUT
{
    float4 posH : SV_Position;
    float2 texcoord   : TEXCOORD;
};

// ガウシアンブラー構造体
struct GaussianBlur
{
    float2 texelSize;
    float2 direction;  // ブラーの方向（例：水平なら(1,0)、垂直なら(0,1)）
    float  blur;        // ブラーの強さ（例：1.0f）
    float3 padding;
    
    float4 weights; // ブラーの重み
    float4 offsets;
};
cbuffer GaussianBlurBuffer : register(b0) {
    GaussianBlur g_GaussianBlur;
}

// main関数
float4 main(PS_INPUT ps_in) : SV_TARGET
{
    const float centerWeight = 0.264151f;
    const float combinedWeight = 0.367925f;
    const float combinedOffset = 1.384615f;

    float2 texOffset = g_GaussianBlur.direction
        * g_GaussianBlur.texelSize
        * combinedOffset
        * g_GaussianBlur.blur;

    float4 color = g_Texture.Sample(g_SamplerState, ps_in.texcoord) * centerWeight;
    color += g_Texture.Sample(g_SamplerState, ps_in.texcoord + texOffset) * combinedWeight;
    color += g_Texture.Sample(g_SamplerState, ps_in.texcoord - texOffset) * combinedWeight;
    return color;
}

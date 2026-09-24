// mosaic_ps.hlsl

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

// 入力用構造体
struct PS_INPUT
{
    float4 posH : SV_Position;
    float2 texcoord   : TEXCOORD;
};

cbuffer MosaicBuffer : register(b0) {
    float2 g_ScreenSize; // 画面サイズ
    float g_MosaicSize;  // 1ブロックのピクセル数
    float g_Strength;    // モザイクの強さ（0～1）
};

float4 main(PS_INPUT ps_in) : SV_TARGET
{
    float2 screenSize = max(g_ScreenSize, float2(1.0f, 1.0f));
    float mosaicSize = max(g_MosaicSize, 1.0f);
    
    float2 mosaicUv = floor(ps_in.texcoord * screenSize / mosaicSize) * mosaicSize / screenSize;
    mosaicUv = clamp(mosaicUv, 0.001f, 0.999f);

    float4 originalColor = g_Texture.Sample(g_SamplerState, ps_in.texcoord);
    float4 mosaicColor = g_Texture.Sample(g_SamplerState, mosaicUv);
    return lerp(originalColor, mosaicColor, saturate(g_Strength));
}

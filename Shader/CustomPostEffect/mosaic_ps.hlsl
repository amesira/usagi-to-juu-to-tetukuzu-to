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
    float2 screenSize; // 画面サイズ
    float2 mosaicSize; // モザイクのサイズ
};

float4 main(PS_INPUT ps_in) : SV_TARGET
{
    float uv = ps_in.texcoord;
    
    float2 len = float2(0.5f, 0.5f);
    len -= ps_in.texcoord;
    len.x *= screenSize.x / screenSize.y;
    len = length(len);
    
    if (len.x < 0.0f)
    {
        uv *= screenSize;
        uv /= mosaicSize;
        uv = floor(uv) * mosaicSize;
        uv /= screenSize;
    }
    
    // テクスチャのサンプリング
    uv = clamp(uv, 0.001f, 0.999f);
    return g_Texture.Sample(g_SamplerState, uv);
}
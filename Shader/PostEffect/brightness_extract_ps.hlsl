//+++++++++++++++++++++++++++++++++++++++++++++++++++
// brightness_extract_ps.hlsl
// ・明るさ抽出のピクセルシェーダー
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

cbuffer BrightnessExtractBuffer : register(b0) {
    float g_BrightnessThreshold; // 明るさの閾値
};

float4 main(PS_INPUT ps_in) : SV_TARGET
{
    float4 color = g_Texture.Sample(g_SamplerState, ps_in.texcoord);
    
    // 輝度を計算
    float Y = dot(color.rgb, float3(0.299f, 0.587f, 0.114f));
    
    // 閾値チェック
    if (Y <= g_BrightnessThreshold) {
        color = float4(0.0f, 0.0f, 0.0f, 1.0f); // 閾値以下は黒にする
    }
    
    return color;
}
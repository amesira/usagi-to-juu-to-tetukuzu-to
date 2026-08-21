//+++++++++++++++++++++++++++++++++++++++++++++++++++
// File  ：Shader/mesh_effect_unlit_ps.hlsl
// Date  ：2026/08/21
// Author：Miu Kitamura
// 
// ・MeshEffect用のUnlitピクセルシェーダー
//+++++++++++++++++++++++++++++++++++++++++++++++++++
Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

struct PS_INPUT
{
    float4 posH     : SV_Position;  // 変換済み頂点座標
    float4 posW     : POSITION1;    // ワールド座標
    float4 normal   : NORMAL0;      // 頂点法線
    float4 color    : COLOR0;       // 頂点カラー
    float2 texcoord : TEXCOORD0;    // テクスチャ座標
};

cbuffer MeshEffectPixelBuffer : register(b9)
{
    // CPUでGradientModule::colorを評価した結果
    float4 g_EffectColor;

    // Flipbookを含めてCPUで計算した最終UV領域
    // xy: 左上、zw: 幅・高さ
    float4 g_FrameUVRect;

    // ScrollModule
    float2 g_UVTiling;
    float2 g_UVOffset;

    // UV Wave
    float2 g_UVWaveDirection;
    float  g_UVWaveAmplitude;
    float  g_UVWaveFrequency;

    float  g_UVWaveSpeed;
    float  g_EffectTime;
    float  g_AlphaCutoff;
};

float4 main(PS_INPUT ps_in) : SV_TARGET
{
    float2 uv = ps_in.texcoord;

    // UVにTiling・Scrollを適用
    uv = uv * g_UVTiling + g_UVOffset;

    // Waveを適用
    const float phase = dot(uv, g_UVWaveDirection) * g_UVWaveFrequency + g_EffectTime * g_UVWaveSpeed;
    uv += g_UVWaveDirection * sin(phase) * g_UVWaveAmplitude;

    // 最後にアトラス上の対象領域へ変換
    uv = g_FrameUVRect.xy + uv * g_FrameUVRect.zw;

    float4 color = g_Texture.Sample(g_SamplerState, uv) * ps_in.color * g_EffectColor;

    if (color.a <= g_AlphaCutoff) discard;
    return color;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++
// ui_vs.hlsl
// 
// Author：Miu Kitamura
// Date  ：2026/04/01
//+++++++++++++++++++++++++++++++++++++++++++++++++++
#include "./Common/transform.hlsl"
#include "./Common/camera.hlsl"

// 入力用頂点構造体 [Ui Vertex]
struct VS_INPUT
{
    float4 posL     : POSITION0;    // 頂点座標
    float2 texcoord : TEXCOORD0;    // テクスチャ座標（U,V）
    
    float4 world0    : WORLD0;
    float4 world1    : WORLD1;
    float4 world2    : WORLD2;
    float4 world3    : WORLD3;
    float4 color     : COLOR0;
    float4 uvRect    : TEXCOORD1;
    float4 roundFill : TEXCOORD2;
};

// 出力用頂点構造体 [Ui Vertex]
struct VS_OUTPUT
{
    float4 posH     : SV_Position; // 変換済み頂点座標
    float4 color    : COLOR0;       // 頂点カラー
    float2 texcoord : TEXCOORD0;    // テクスチャ座標
    float2 localUv : TEXCOORD1;
    
    // 補間を行わないパラメータ
    nointerpolation float4 roundFill : TEXCOORD2;
};

// main関数
VS_OUTPUT main(VS_INPUT vs_in)
{
    VS_OUTPUT vs_out;
    
    // ワールド行列を構築
    float4x4 world = float4x4(
        vs_in.world0,
        vs_in.world1,
        vs_in.world2,
        vs_in.world3
    );

    // 頂点を行列変換
    float4 posW = mul(float4(vs_in.posL.xyz, 1.0f), world);
    vs_out.posH = mul(mul(posW, g_ViewMatrix), g_ProjectionMatrix);
    vs_out.color = vs_in.color;
    vs_out.localUv = vs_in.texcoord;
    vs_out.roundFill = vs_in.roundFill;
    vs_out.texcoord = vs_in.uvRect.xy + vs_in.texcoord * vs_in.uvRect.zw;
    
    return vs_out;
}
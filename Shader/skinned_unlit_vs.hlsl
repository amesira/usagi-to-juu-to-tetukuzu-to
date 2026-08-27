//+++++++++++++++++++++++++++++++++++++++++++++++++++
// skinned_unlit_vs.hlsl [Skinned Unlit vertex shader]
//+++++++++++++++++++++++++++++++++++++++++++++++++++
#include "./Common/transform.hlsl"
#include "./Common/camera.hlsl"
#include "./Common/skinning.hlsl"

struct VS_INPUT
{
    float4 posL       : POSITION0;
    float4 normal     : NORMAL0;
    float4 tangent    : TANGENT0;
    float4 binormal   : BINORMAL0;
    float4 color      : COLOR0;
    float2 texcoord   : TEXCOORD0;
    uint4 boneIndex   : BLENDINDICES0;
    float4 boneWeight : BLENDWEIGHT0;
};

struct VS_OUTPUT
{
    float4 posH     : SV_Position;
    float4 posW     : POSITION1;
    float4 normal   : NORMAL0;
    float4 color    : COLOR0;
    float2 texcoord : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT vs_in)
{
    VS_OUTPUT vs_out;

    float4x4 skinMatrix = CalcSkinningMatrix(vs_in.boneIndex, vs_in.boneWeight);
    float4 skinPosL = mul(vs_in.posL, skinMatrix);
    vs_out.posW = mul(skinPosL, g_WorldMatrix);
    vs_out.posH = mul(mul(vs_out.posW, g_ViewMatrix), g_ProjectionMatrix);

    float3 skinnedNormal = mul(vs_in.normal.xyz, (float3x3)skinMatrix);
    vs_out.normal = float4(normalize(mul(skinnedNormal, (float3x3)g_WorldMatrix)), 0.0f);
    vs_out.color = vs_in.color;
    vs_out.texcoord = vs_in.texcoord;

    return vs_out;
}

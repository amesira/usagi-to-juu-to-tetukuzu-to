#include "./Common/camera.hlsl"

struct VS_INPUT
{
    float3 position : POSITION0;
    float4 color : COLOR0;
};

struct VS_OUTPUT
{
    float4 position : SV_Position;
    float4 color : COLOR0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    // Debug line endpoints are supplied in world space.
    float4 position = float4(input.position, 1.0f);
    output.position = mul(mul(position, g_ViewMatrix), g_ProjectionMatrix);
    output.color = input.color;
    return output;
}

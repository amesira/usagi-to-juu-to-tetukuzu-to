// gaussian_v_ps.hlsl
// 横向きブラー用PS
Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

struct PS_INPUT
{
    float4 posH : SV_Position;
    float2 texcoord : TEXCOORD;
};

cbuffer GaussianBuffer : register(b0) {
    float4 g_GaussianWeights; // ガウシアンの重み
    float4 g_GaussianWeights2; // ガウシアンの重み2
    float g_Strength; // ブラーの強さ
    
    float3 padding;
};

float4 main(PS_INPUT ps_in) : SV_TARGET
{
    float weights[9] = { 
        0.0f,
        g_GaussianWeights.x, g_GaussianWeights.y, g_GaussianWeights.z, g_GaussianWeights.w,
        g_GaussianWeights2.x, g_GaussianWeights2.y, g_GaussianWeights2.z, g_GaussianWeights2.w
    };
    
    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float2 offset = float2(x, y) * g_Strength;
            color += g_Texture.Sample(g_SamplerState, ps_in.texcoord + offset) * weights[(x + 1) * 3 + (y + 1)];
        }
    }
    
    return color;
}

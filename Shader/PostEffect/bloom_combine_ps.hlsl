//+++++++++++++++++++++++++++++++++++++++++++++++++++
// bloom_combine_ps.hlsl
//+++++++++++++++++++++++++++++++++++++++++++++++++++
Texture2D g_SceneTexture : register(t0);
Texture2D g_BloomTexture0 : register(t1);
Texture2D g_BloomTexture1 : register(t2);
Texture2D g_BloomTexture2 : register(t3);
Texture2D g_BloomTexture3 : register(t4);
SamplerState g_SamplerState : register(s0);

struct PS_INPUT
{
    float4 posH : SV_Position;
    float2 texcoord : TEXCOORD;
};

float4 main(PS_INPUT ps_in) : SV_TARGET
{
    float4 sceneColor = g_SceneTexture.Sample(g_SamplerState, ps_in.texcoord);
    float3 bloom = g_BloomTexture0.Sample(g_SamplerState, ps_in.texcoord).rgb;
    bloom += g_BloomTexture1.Sample(g_SamplerState, ps_in.texcoord).rgb;
    bloom += g_BloomTexture2.Sample(g_SamplerState, ps_in.texcoord).rgb;
    bloom += g_BloomTexture3.Sample(g_SamplerState, ps_in.texcoord).rgb;
    return float4(sceneColor.rgb + bloom * 0.25f, sceneColor.a);
}

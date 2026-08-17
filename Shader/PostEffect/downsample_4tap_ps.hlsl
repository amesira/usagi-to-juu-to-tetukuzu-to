//+++++++++++++++++++++++++++++++++++++++++++++++++++
// downsample_4tap_ps.hlsl
//+++++++++++++++++++++++++++++++++++++++++++++++++++
Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

struct PS_INPUT
{
    float4 posH : SV_Position;
    float2 texcoord : TEXCOORD;
};

float4 main(PS_INPUT ps_in) : SV_TARGET
{
    // 出力が入力の1/2サイズの場合、線形補間の1サンプルで入力2x2の平均になる
    return g_Texture.Sample(g_SamplerState, ps_in.texcoord);
}

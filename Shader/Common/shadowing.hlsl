//+++++++++++++++++++++++++++++++++++++++++++++++++++
// shadowing.hlsl
// ・シャドウマッピング用のシェーダーファイル
// ・-b11-にシャドウ用ライト行列定数バッファを割り当てる
// ・-t10-にシャドウマップテクスチャを割り当てる
// 
// Author：Miu Kitamura
// Date  ：2026/04/03
//+++++++++++++++++++++++++++++++++++++++++++++++++++

// シャドウ用ライト行列
cbuffer ShadowMatrixBuffer : register(b11)
{
    float4x4 g_LightViewProjMatrix; // ライトのビュー射影行列
};

// シャドウマップ用のサンプラー
SamplerComparisonState g_ShadowSampler : register(s1);

// シャドウマップテクスチャ
Texture2D g_ShadowMap : register(t10);

// ワールド座標をライト空間座標に変換
float4 WorldToLightSpace(float4 posW)
{
    return mul(posW, g_LightViewProjMatrix);
}

// ライト空間座標からシャドウマップのUV座標を計算
float2 CalcShadowUV(float4 lightWorldPos)
{
    // ライト空間座標をNDC空間に変換（範囲外は範囲外として扱いたいのでclampはしない）
    float2 shadowUV = lightWorldPos.xy / lightWorldPos.w * 0.5f + 0.5f;
    shadowUV.y = 1.0f - shadowUV.y;
    return shadowUV;
}

// シャドウマップの深度値を取得
float GetShadowDepth(float2 shadowUV, float depthInLightSpace)
{
    float sampledDepth = 0.0f;
    
    // 3x3サンプリング
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float2 offsetUV = shadowUV + float2(x, y) * 0.001f;
            sampledDepth += g_ShadowMap.SampleCmpLevelZero(
                g_ShadowSampler,
                offsetUV,
                depthInLightSpace - 0.001f).r;
        }
    }
    
    return sampledDepth / 9.0f; // 平均値を返す
}
//===================================================
// File  ：_/RenderPass/PostEffect/bloom_effect.cpp
// Date  ：2026/08/17
// Author：Miu Kitamura
// 
// ・PostEffectのブルームエフェクトを実装する
//===================================================
#include "bloom_effect.h"

#include <algorithm>
#include <cstring>
#include "Engine/engine_service_locator.h"

/// @brief ブルームエフェクトを初期化する
void BloomEffect::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    m_device = device;
    m_context = context;

    const unsigned int screenWidth = Direct3D_GetBackBufferWidth();
    const unsigned int screenHeight = Direct3D_GetBackBufferHeight();

    // ダウンサンプリング用のテクスチャとレンダーターゲットを作成する
    for (int level = 0; level < DownsampleLevelCount; level++) {
        m_downsampledWidth[level] = screenWidth >> (level + 1);
        m_downsampledHeight[level] = screenHeight >> (level + 1);

        const int bufferCount = (level == 0) ? 1 : 2;
        for (int bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++) {
            const int resourceIndex = level * 2 + bufferIndex;
            Direct3D_CreateColorBuffer(
                m_downsampledTexture[resourceIndex].GetAddressOf(),
                m_downsampledRTV[resourceIndex].GetAddressOf(),
                m_downsampledSRV[resourceIndex].GetAddressOf(),
                m_downsampledWidth[level],
                m_downsampledHeight[level],
                DXGI_FORMAT_R16G16B16A16_FLOAT);
        }
    }

    // ブルームエフェクト用の定数バッファを作成する
    m_constantBuffer = SHADER_REPOSITORY->GenerateConstantBufferResource(
        "BloomEffectBuffer", 0, sizeof(ConstantBufferData), false, true, ConstantBufferUsage::Dynamic);

    // ブルームエフェクト用のシェーダーを作成する
    ShaderProgramResource* fullScreenShader =
        SHADER_REPOSITORY->GetShaderProgramResource(ShaderBase::FullScreen);

    ShaderProgramResource brightnessShaderResource;
    brightnessShaderResource.name = "BloomBrightnessExtract";
    brightnessShaderResource.baseShader = fullScreenShader;
    brightnessShaderResource.overridePixelShader =
        SHADER_REPOSITORY->GetPixelShaderResource("brightness_extract_ps.cso");
    brightnessShaderResource.additionalConstantBuffers.push_back(m_constantBuffer);
    m_brightnessExtractShader =
        SHADER_REPOSITORY->GenerateShaderProgramResource(brightnessShaderResource);

    ShaderProgramResource gaussianShaderResource;
    gaussianShaderResource.name = "BloomGaussianBlur";
    gaussianShaderResource.baseShader = fullScreenShader;
    gaussianShaderResource.overridePixelShader =
        SHADER_REPOSITORY->GetPixelShaderResource("gaussian_blur_ps.cso");
    gaussianShaderResource.additionalConstantBuffers.push_back(m_constantBuffer);
    m_gaussianBlurShader =
        SHADER_REPOSITORY->GenerateShaderProgramResource(gaussianShaderResource);

    ShaderProgramResource combineShaderResource;
    combineShaderResource.name = "BloomCombineEffect";
    combineShaderResource.baseShader = fullScreenShader;
    combineShaderResource.overridePixelShader =
        SHADER_REPOSITORY->GetPixelShaderResource("bloom_combine_ps.cso");
    m_combineShader = SHADER_REPOSITORY->GenerateShaderProgramResource(combineShaderResource);
}

/// @brief ブルームエフェクトを破棄する
void BloomEffect::Process(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV)
{
    if (!m_context || !inputSRV || !outputRTV || !m_constantBuffer) return;

    SetSamplerState(SAMPLERSTATE_LINEAR_CLAMP);

    SetBlendState(BLENDSTATE_NONE);
    SetDepthState(DEPTHSTATE_DISABLE);

    // 1. HDR入力を1/2へ縮小しながら輝度抽出する。
    Direct3D_SetViewport(m_downsampledWidth[0], m_downsampledHeight[0]);
    Direct3D_SetSceneTarget(m_downsampledRTV[0].Get(), nullptr);

    EngineServiceLocator::BindShader(m_brightnessExtractShader);
    UpdateBrightnessConstantBuffer(2.0f);

    m_context->PSSetShaderResources(0, 1, &inputSRV);
    m_context->Draw(3, 0);
    UnbindShaderResources(0, 1);

    // 2. 1/4以降を縮小し、横・縦の3サンプルガウシアンブラーを適用する。（川瀬式）
    for (int level = 1; level < DownsampleLevelCount; level++) {
        Direct3D_SetViewport(m_downsampledWidth[level], m_downsampledHeight[level]);

        const int workIndex1 = level * 2;       // ワークテクスチャ1
        const int workIndex2 = level * 2 + 1;   // ワークテクスチャ2
        const int previousResultIndex = (level - 1) * 2;

        // === ダウンサンプリング ===
        Direct3D_SetSceneTarget(m_downsampledRTV[workIndex1].Get(), nullptr);

        // 前の結果をより解像度の小さいテクスチャへ縮小する（ぼやかしはサンプラーステート頼り）
        EngineServiceLocator::BindShader(ShaderBase::FullScreen);
        m_context->PSSetShaderResources(
            0, 1, m_downsampledSRV[previousResultIndex].GetAddressOf());
        m_context->Draw(3, 0);

        // === ガウシアンブラー ===
        XMFLOAT2 texelSize = { 1.0f / m_downsampledWidth[level], 1.0f / m_downsampledHeight[level] };

        // 横方向のブラー
        Direct3D_SetSceneTarget(m_downsampledRTV[workIndex2].Get(), nullptr);
        EngineServiceLocator::BindShader(m_gaussianBlurShader);
        UpdateGaussianBlurConstantBuffer(
            texelSize,
            XMFLOAT2(1.0f, 0.0f),
            1.0f);
        m_context->PSSetShaderResources(0, 1, m_downsampledSRV[workIndex1].GetAddressOf());
        m_context->Draw(3, 0);

        // 縦方向のブラー
        Direct3D_SetSceneTarget(m_downsampledRTV[workIndex1].Get(), nullptr);
        UpdateGaussianBlurConstantBuffer(
            texelSize,
            XMFLOAT2(0.0f, 1.0f),
            1.0f);
        m_context->PSSetShaderResources(0, 1, m_downsampledSRV[workIndex2].GetAddressOf());
        m_context->Draw(3, 0);
        UnbindShaderResources(0, 1);
    }

    // === 最終合成 ===
    Direct3D_ResetViewport();
    Direct3D_SetSceneTarget(outputRTV, nullptr);

    // HDR入力と全Bloomレベルを最終出力へ合成する。
    EngineServiceLocator::BindShader(m_combineShader);
    m_context->PSSetShaderResources(0, 1, &inputSRV);
    for (int level = 0; level < DownsampleLevelCount; level++) {
        const int resultIndex = level * 2;
        m_context->PSSetShaderResources(
            level + 1, 1, m_downsampledSRV[resultIndex].GetAddressOf());
    }
    m_context->Draw(3, 0);
    UnbindShaderResources(0, DownsampleLevelCount + 1);
}

// ----------

/// @brief ブルームエフェクトの明るさ抽出用定数バッファを更新する
void BloomEffect::UpdateBrightnessConstantBuffer(float threshold)
{
    m_constantBufferData = {};
    m_constantBufferData.brightnessExtract.threshold = threshold;
    
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (SUCCEEDED(m_context->Map(
        m_constantBuffer->buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        std::memcpy(mapped.pData, &m_constantBufferData, sizeof(m_constantBufferData));
        m_context->Unmap(m_constantBuffer->buffer.Get(), 0);
    }
}

/// @brief ブルームエフェクトのガウシアンブラー用定数バッファを更新する
void BloomEffect::UpdateGaussianBlurConstantBuffer(
    const XMFLOAT2& texelSize, const XMFLOAT2& direction, float blur)
{
    m_constantBufferData = {};
    m_constantBufferData.gaussianBlur.texelSize = texelSize;
    m_constantBufferData.gaussianBlur.direction = direction;
    m_constantBufferData.gaussianBlur.blur = blur;

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (SUCCEEDED(m_context->Map(
        m_constantBuffer->buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        std::memcpy(mapped.pData, &m_constantBufferData, sizeof(m_constantBufferData));
        m_context->Unmap(m_constantBuffer->buffer.Get(), 0);
    }
}

/// @brief シェーダーリソースビューを解除する
void BloomEffect::UnbindShaderResources(UINT startSlot, UINT count)
{
    ID3D11ShaderResourceView* nullSRVs[8] = {};
    const UINT safeCount = (std::min)(count, static_cast<UINT>(8));
    m_context->PSSetShaderResources(startSlot, safeCount, nullSRVs);
}

//===================================================
// post_process.cpp
// 
// Author: Miu Kitamura
// Date  : 2026/04/29
//===================================================
#include "post_process.h"

#include <algorithm>

#include "Engine/engine_service_locator.h"
#define SHADER_REPOSITORY EngineServiceLocator::GetShaderRepository()

void PostProcess::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    m_device = device;
    m_context = context;

    // 画面サイズに合わせた一時バッファを作成する
    unsigned int SCREEN_WIDTH = Direct3D_GetBackBufferWidth();
    unsigned int SCREEN_HEIGHT = Direct3D_GetBackBufferHeight();

    // Bloom用に 1/2, 1/4, 1/8, 1/16 の縮小バッファを用意する
    // 1/2は輝度抽出結果のみ、以降は縮小結果と縦横ブラー用の2枚を使い回す
    for (int level = 0; level < static_cast<int>(DownsampleLevel::MAX); level++) {
        m_downsampledWidth[level] = SCREEN_WIDTH >> (level + 1);
        m_downsampledHeight[level] = SCREEN_HEIGHT >> (level + 1);

        const int bufferCount = (level == 0) ? 1 : 2;
        for (int i = 0; i < bufferCount; i++) {
            Direct3D_CreateColorBuffer(
                m_downsampledTexture[level * 2 + i].GetAddressOf(),
                m_downsampledRTV[level * 2 + i].GetAddressOf(),
                m_downsampledSRV[level * 2 + i].GetAddressOf(),
                m_downsampledWidth[level],
                m_downsampledHeight[level],
                DXGI_FORMAT_R16G16B16A16_FLOAT
            );
        }
    }

    // ポストエフェクト用の定数バッファをフルスクリーン描画シェーダーへ登録する
    m_postProcessCB = SHADER_REPOSITORY->GenerateConstantBufferResource(
        "PostProcessBuffer",
        0,
        sizeof(PostProcessBuffer),
        true,
        true,
        ConstantBufferUsage::Dynamic);
    SHADER_REPOSITORY->AddConstantBufferToShaderProgram(SHADER_BASE_NAMES[static_cast<size_t>(ShaderBase::FullScreen)], m_postProcessCB);

    m_fullScreenShader = SHADER_REPOSITORY->GetShaderProgramResource(ShaderBase::FullScreen);

    // 輝度抽出：Bloomの元になる明るい部分だけを取り出す
    ShaderProgramResource brightnessExtractShaderResource;
    brightnessExtractShaderResource.name = "BrightnessExtract";
    brightnessExtractShaderResource.baseShader = m_fullScreenShader;
    brightnessExtractShaderResource.overridePixelShader = SHADER_REPOSITORY->GetPixelShaderResource("brightness_extract_ps.cso");
    m_brightnessExtractShader = SHADER_REPOSITORY->GenerateShaderProgramResource(brightnessExtractShaderResource);

    // ガウスブラー：縮小後のBloom素材を水平・垂直の2passでぼかす
    ShaderProgramResource gaussianBlurShaderResource;
    gaussianBlurShaderResource.name = "GaussianBlur";
    gaussianBlurShaderResource.baseShader = m_fullScreenShader;
    gaussianBlurShaderResource.overridePixelShader = SHADER_REPOSITORY->GetPixelShaderResource("gaussian_blur_ps.cso");
    m_gaussianBlurShader = SHADER_REPOSITORY->GenerateShaderProgramResource(gaussianBlurShaderResource);

    // Bloom合成：各縮小レベルのBloom素材を1枚にまとめる
    ShaderProgramResource bloomCombineShaderResource;
    bloomCombineShaderResource.name = "BloomCombine";
    bloomCombineShaderResource.baseShader = m_fullScreenShader;
    bloomCombineShaderResource.overridePixelShader = SHADER_REPOSITORY->GetPixelShaderResource("bloom_combine_ps.cso");
    m_bloomCombineShader = SHADER_REPOSITORY->GenerateShaderProgramResource(bloomCombineShaderResource);

    // トーンマッピング：HDRレンダー結果を表示用の色域へ変換する
    ShaderProgramResource toneMappingShaderResource;
    toneMappingShaderResource.name = "ToneMapping";
    toneMappingShaderResource.baseShader = m_fullScreenShader;
    toneMappingShaderResource.overridePixelShader = SHADER_REPOSITORY->GetPixelShaderResource("tone_mapping_ps.cso");
    m_toneMappingShader = SHADER_REPOSITORY->GenerateShaderProgramResource(toneMappingShaderResource);
}

void PostProcess::Finalize()
{
}

void PostProcess::Process(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV)
{
    // 1. Bloom処理
    Bloom(inputSRV, outputRTV);

    SetSamplerState(SAMPLERSTATE_POINT_WRAP);
}

void PostProcess::ToneMapping(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV)
{
    // HDRの入力テクスチャをトーンマッピングして出力RTVへ描画する
    EngineServiceLocator::BindShader(m_toneMappingShader);
    SetBlendState(BLENDSTATE_NONE);
    SetDepthState(DEPTHSTATE_DISABLE);
    m_context->PSSetShaderResources(0, 1, &inputSRV);

    Direct3D_ResetViewport();
    Direct3D_ClearSceneTarget(outputRTV, nullptr);
    Direct3D_SetSceneTarget(outputRTV, nullptr);

    m_context->Draw(3, 0);
    UnbindShaderResources(0, 1);
}

/// @brief Bloom処理を行う
void PostProcess::Bloom(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV)
{
    SetSamplerState(SAMPLERSTATE_LINEAR_CLAMP);

    SetBlendState(BLENDSTATE_NONE);
    SetDepthState(DEPTHSTATE_DISABLE);

    // 1. HDR入力を1/2へダウンサンプリングしながら輝度抽出する
    Direct3D_SetViewport(m_downsampledWidth[0], m_downsampledHeight[0]);
    Direct3D_SetSceneTarget(m_downsampledRTV[0].Get(), nullptr);
    EngineServiceLocator::BindShader(m_brightnessExtractShader);

    m_postProcessBufferData.Reset();
    {
        auto& be = m_postProcessBufferData.brightnessExtract;
        be.threshold = 2.0f;
    }
    UpdateConstantBuffer();

    m_context->PSSetShaderResources(0, 1, &inputSRV);
    m_context->Draw(3, 0);
    UnbindShaderResources(0, 1);

    // 2. 1/4以降をダウンサンプルし、3サンプルの横・縦ガウシアンブラーをかける
    for (int i = 1; i < static_cast<int>(DownsampleLevel::MAX); i++) {
        Direct3D_SetViewport(m_downsampledWidth[i], m_downsampledHeight[i]);

        // 2-a. 前レベルの結果を1段小さいバッファへ縮小する
        Direct3D_SetSceneTarget(m_downsampledRTV[i * 2].Get(), nullptr);
        EngineServiceLocator::BindShader(ShaderBase::FullScreen);
        const int previousResultIndex = (i - 1) * 2;
        m_context->PSSetShaderResources(0, 1, m_downsampledSRV[previousResultIndex].GetAddressOf());
        m_context->Draw(3, 0);
        UnbindShaderResources(0, 1);

        // 2-b. 横方向のガウシアンブラーをかける
        const int horizontalInputIndex = i * 2;
        const int horizontalOutputIndex = i * 2 + 1;
        Direct3D_SetSceneTarget(m_downsampledRTV[horizontalOutputIndex].Get(), nullptr);

        EngineServiceLocator::BindShader(m_gaussianBlurShader);
        m_postProcessBufferData.Reset();
        {
            auto& gb = m_postProcessBufferData.gaussianBlur;
            gb.texelSize = XMFLOAT2(1.0f / m_downsampledWidth[i], 1.0f / m_downsampledHeight[i]);
            gb.direction = XMFLOAT2(1, 0);
            gb.blur = 1.0f;
        }
        UpdateConstantBuffer();

        m_context->PSSetShaderResources(0, 1, m_downsampledSRV[horizontalInputIndex].GetAddressOf());
        m_context->Draw(3, 0);
        UnbindShaderResources(0, 1);

        // 2-c. 横ブラーの結果へ縦方向のガウシアンブラーをかける
        const int verticalOutputIndex = i * 2;
        Direct3D_SetSceneTarget(m_downsampledRTV[verticalOutputIndex].Get(), nullptr);

        {
            auto& gb = m_postProcessBufferData.gaussianBlur;
            gb.direction = XMFLOAT2(0, 1);
        }
        UpdateConstantBuffer();

        m_context->PSSetShaderResources(0, 1, m_downsampledSRV[horizontalOutputIndex].GetAddressOf());
        m_context->Draw(3, 0);
        UnbindShaderResources(0, 1);
    }

    // 3. 元のHDR画像と各Bloomレベルを最終出力へ1パスで合成する
    Direct3D_ResetViewport();
    Direct3D_SetSceneTarget(outputRTV, nullptr);

    EngineServiceLocator::BindShader(m_bloomCombineShader);
    m_context->PSSetShaderResources(0, 1, &inputSRV);

    for (int i = 0; i < static_cast<int>(DownsampleLevel::MAX); i++) {
        const int resultIndex = i * 2;
        m_context->PSSetShaderResources(i + 1, 1, m_downsampledSRV[resultIndex].GetAddressOf());
    }
    m_context->Draw(3, 0);
    UnbindShaderResources(0, static_cast<UINT>(DownsampleLevel::MAX) + 1);
}

void PostProcess::UpdateConstantBuffer()
{
    // union内の現在有効なパラメータをそのまま定数バッファへ転送する
    D3D11_MAPPED_SUBRESOURCE msr = {};
    m_context->Map(m_postProcessCB->buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    XMFLOAT4* cbData = reinterpret_cast<XMFLOAT4*>(msr.pData);
    for (int i = 0; i < 8; i++) {
        cbData[i] = m_postProcessBufferData.data[i];
    }
    m_context->Unmap(m_postProcessCB->buffer.Get(), 0);
}

void PostProcess::UnbindShaderResources(UINT startSlot, UINT count)
{
    // 次のpassで同じテクスチャをRTVとして使うため、PS側のSRV参照を明示的に外す
    ID3D11ShaderResourceView* nullSRV[8] = {};
    const UINT safeCount = (std::min)(count, static_cast<UINT>(8));
    m_context->PSSetShaderResources(startSlot, safeCount, nullSRV);
}

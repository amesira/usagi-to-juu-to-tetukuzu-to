//===================================================
// post_effect_pass.cpp
//
// Author: Miu Kitamura
// Date  : 2026/04/29
//===================================================
#include "post_effect_pass.h"

#include "Engine/Core/scene_interface.h"
#include "Engine/render_view.h"
#include "Engine/Asset/EnvironmentAsset/environment_asset.h"
#include "Engine/Core/scene_post_effect_state.h"

namespace {
    void Swap(int& prevCount, int& count) {
        prevCount = count;
        count = (count + 1) % 2;
    }
}

// ポストエフェクト初期化
void PostEffectPass::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    m_pDevice = pDevice;
    m_pContext = pContext;

    // 画面サイズに合わせた一時バッファを作成する
    unsigned int SCREEN_WIDTH = Direct3D_GetBackBufferWidth();
    unsigned int SCREEN_HEIGHT = Direct3D_GetBackBufferHeight();

    for (int i = 0; i < 2; i++) {
        Direct3D_CreateColorBuffer(
            m_tempTexture[i].GetAddressOf(),
            m_tempRTV[i].GetAddressOf(),
            m_tempSRV[i].GetAddressOf(),
            SCREEN_WIDTH,
            SCREEN_HEIGHT
        );
    }

    m_bloomEffect.Initialize(m_pDevice, m_pContext);
    m_monoMaskEffect.Initialize(m_pDevice, m_pContext);
    m_radialBlurEffect.Initialize(m_pDevice, m_pContext);
    m_mosaicEffect.Initialize(m_pDevice, m_pContext);
    m_chromaticAberrationEffect.Initialize(m_pDevice, m_pContext);
    m_posterizeEffect.Initialize(m_pDevice, m_pContext);
    m_horrorNoiseEffect.Initialize(m_pDevice, m_pContext);
}

// ポストエフェクト終了
void PostEffectPass::Finalize()
{
}

// ポストエフェクト処理
void PostEffectPass::Process(IScene* pScene, const RenderView& view)
{
    const EnvironmentPostProcessData& environment =
        pScene->GetEnvironmentAsset().GetData().postProcess;
    const CustomPostEffectState& state = pScene->GetPostEffectState();

    int prevCount = 0;
    int count = 0;

    // 1. Bloom
    m_bloomEffect.Process(
        view.colorBufferSRV.Get(), m_tempRTV[count].Get(), environment.bloom);
    Swap(prevCount, count);

    // 2. MonoMask
    if (state.monoMask.strength > 0.01f) {
        ID3D11ShaderResourceView* maskSRV = view.maskColorBufferSRV
            ? view.maskColorBufferSRV.Get()
            : state.monoMaskTextureSRV;
        m_monoMaskEffect.Process(
            m_tempSRV[prevCount].Get(),
            m_tempRTV[count].Get(),
            maskSRV,
            state.monoMask.monoColor,
            state.monoMask.strength);
        Swap(prevCount, count);
    }

    // 3. Mosaic
    if (state.mosaic.strength > 0.01f) {
        m_mosaicEffect.Process(
            m_tempSRV[prevCount].Get(), m_tempRTV[count].Get(),
            state.mosaic.mosaicSize, state.mosaic.strength);
        Swap(prevCount, count);
    }

    // 4. Posterize
    if (state.posterize.strength > 0.01f) {
        m_posterizeEffect.Process(
            m_tempSRV[prevCount].Get(), m_tempRTV[count].Get(),
            state.posterize.levels, state.posterize.strength);
        Swap(prevCount, count);
    }

    // 5. HorrorNoise
    if (state.horrorNoise.strength > 0.01f) {
        m_horrorNoiseEffect.Process(
            m_tempSRV[prevCount].Get(), m_tempRTV[count].Get(),
            state.horrorNoise.noiseMin, state.horrorNoise.noiseMax,
            state.horrorNoise.contrastPow, state.horrorNoise.strength,
            state.horrorNoise.time);
        Swap(prevCount, count);
    }

    // 6. ChromaticAberration
    if (state.chromaticAberration.strength > 0.01f) {
        m_chromaticAberrationEffect.Process(
            m_tempSRV[prevCount].Get(), m_tempRTV[count].Get(),
            state.chromaticAberration.redShiftPixels,
            state.chromaticAberration.blueShiftPixels,
            state.chromaticAberration.strength);
        Swap(prevCount, count);
    }

    // 7. RadialBlur
    m_radialBlurEffect.Process(
        m_tempSRV[prevCount].Get(),
        view.postEffectRTV.Get(),
        state.radialBlur.sampleCount,
        state.radialBlur.strength);

    SetSamplerState(SAMPLERSTATE_POINT_WRAP);
}

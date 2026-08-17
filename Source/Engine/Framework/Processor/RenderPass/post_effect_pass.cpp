//===================================================
// post_effect_pass.cpp
//
// Author: Miu Kitamura
// Date  : 2026/04/29
//===================================================
#include "post_effect_pass.h"

#include "Engine/Core/scene_interface.h"
#include "Engine/render_view.h"
#include "Engine/Settings/scene_settings.h"

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
}

// ポストエフェクト終了
void PostEffectPass::Finalize()
{
}

// ポストエフェクト処理
void PostEffectPass::Process(IScene* pScene, const RenderView& view)
{
    const CustomPostEffectState& state =
        pScene->GetSceneSettings().GetPostProcessSettings().m_customPostEffectState;

    // 1. Bloom
    m_bloomEffect.Process(view.colorBufferSRV.Get(), m_tempRTV[0].Get());

    // 2. MonoMask（既存の処理順を維持）
    ID3D11ShaderResourceView* maskSRV = view.maskColorBufferSRV
        ? view.maskColorBufferSRV.Get()
        : state.monoMaskTextureSRV;
    m_monoMaskEffect.Process(
        m_tempSRV[0].Get(),
        m_tempRTV[1].Get(),
        maskSRV,
        state.monoMask.monoColor,
        state.monoMask.strength);

    // 3. RadialBlur
    m_radialBlurEffect.Process(
        m_tempSRV[1].Get(),
        view.postEffectRTV.Get(),
        state.radialBlur.sampleCount,
        state.radialBlur.strength);

    SetSamplerState(SAMPLERSTATE_POINT_WRAP);
}

//===================================================
// File  ：_/RenderPass/PostEffect/mono_mask_effect.cpp
// Date  ：2026/08/18
// Author：Miu Kitamura
// 
// ・PostEffectのモノクロマスクエフェクトを実装する
//===================================================
#include "mono_mask_effect.h"

#include "Engine/engine_service_locator.h"

/// @brief モノクロマスクエフェクトを初期化する
void MonoMaskEffect::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    m_context = context;

    // モノクロマスク用の定数バッファを生成
    m_constantBuffer = SHADER_REPOSITORY->GenerateConstantBufferResource(
        "MonoMaskBuffer", 0, sizeof(ConstantBufferData), false, true, ConstantBufferUsage::Dynamic);

    // モノクロマスク用のシェーダーを生成
    ShaderProgramResource shaderResource;
    shaderResource.name = "MonoMaskEffect";
    shaderResource.baseShader = SHADER_REPOSITORY->GetShaderProgramResource(ShaderBase::FullScreen);
    shaderResource.overridePixelShader = SHADER_REPOSITORY->GetPixelShaderResource("mono_mask_ps.cso");
    shaderResource.additionalConstantBuffers.push_back(m_constantBuffer);
    m_monoMaskShader = SHADER_REPOSITORY->GenerateShaderProgramResource(shaderResource);
}

/// @brief モノクロマスクエフェクトを破棄する
/// @param monoColor マスク適用時のモノクロカラー
/// @param strength マスクの強さ（0.0f～1.0f）
void MonoMaskEffect::Process(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV,
    ID3D11ShaderResourceView* maskSRV, const XMFLOAT4& monoColor, float strength)
{
    if (!m_context || !inputSRV || !outputRTV || !m_constantBuffer) return;

    // モノクロマスクの定数バッファを更新
    ConstantBufferData parameters = {};
    parameters.monoColor = monoColor;
    parameters.strength = strength;

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (SUCCEEDED(m_context->Map(m_constantBuffer->buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        *static_cast<ConstantBufferData*>(mapped.pData) = parameters;
        m_context->Unmap(m_constantBuffer->buffer.Get(), 0);
    }

    Direct3D_ResetViewport();
    Direct3D_SetSceneTarget(outputRTV, nullptr);

    // モノクロマスクのシェーダーをバインドして描画
    SetBlendState(BLENDSTATE_NONE);
    SetDepthState(DEPTHSTATE_DISABLE);
    EngineServiceLocator::BindShader(m_monoMaskShader);

    m_context->PSSetShaderResources(0, 1, &inputSRV);
    m_context->PSSetShaderResources(1, 1, &maskSRV);
    m_context->Draw(3, 0);

    // 描画後にシェーダーリソースビューを解除しておく
    ID3D11ShaderResourceView* nullSRVs[2] = {};
    m_context->PSSetShaderResources(0, 2, nullSRVs);
}

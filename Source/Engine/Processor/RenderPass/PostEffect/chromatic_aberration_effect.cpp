#include "chromatic_aberration_effect.h"

#include "Engine/engine_service_locator.h"

void ChromaticAberrationEffect::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    m_context = context;
    m_constantBuffer = SHADER_REPOSITORY->GenerateConstantBufferResource(
        "ChromaticAberrationBuffer", 0, sizeof(ConstantBufferData), false, true, ConstantBufferUsage::Dynamic);

    ShaderProgramResource resource;
    resource.name = "ChromaticAberrationEffect";
    resource.baseShader = SHADER_REPOSITORY->GetShaderProgramResource(ShaderBase::FullScreen);
    resource.overridePixelShader = SHADER_REPOSITORY->GetPixelShaderResource("chromatic_aberration_ps.cso");
    resource.additionalConstantBuffers.push_back(m_constantBuffer);
    m_shader = SHADER_REPOSITORY->GenerateShaderProgramResource(resource);
}

void ChromaticAberrationEffect::Process(ID3D11ShaderResourceView* inputSRV,
    ID3D11RenderTargetView* outputRTV, float redShiftPixels, float blueShiftPixels, float strength)
{
    if (!m_context || !inputSRV || !outputRTV || !m_constantBuffer || !m_shader) return;

    const float width = static_cast<float>(Direct3D_GetBackBufferWidth());
    const float height = static_cast<float>(Direct3D_GetBackBufferHeight());
    ConstantBufferData data = {};
    data.invScreenSize = { width > 0.0f ? 1.0f / width : 0.0f, height > 0.0f ? 1.0f / height : 0.0f };
    data.redShiftPixels = redShiftPixels;
    data.blueShiftPixels = blueShiftPixels;
    data.strength = strength;

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (SUCCEEDED(m_context->Map(m_constantBuffer->buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        *static_cast<ConstantBufferData*>(mapped.pData) = data;
        m_context->Unmap(m_constantBuffer->buffer.Get(), 0);
    }

    SetSamplerState(SAMPLERSTATE_LINEAR_CLAMP);
    Direct3D_ResetViewport();
    Direct3D_SetSceneTarget(outputRTV, nullptr);
    SetBlendState(BLENDSTATE_NONE);
    SetDepthState(DEPTHSTATE_DISABLE);
    EngineServiceLocator::BindShader(m_shader);
    m_context->PSSetShaderResources(0, 1, &inputSRV);
    m_context->Draw(3, 0);
    ID3D11ShaderResourceView* nullSRV = nullptr;
    m_context->PSSetShaderResources(0, 1, &nullSRV);
}

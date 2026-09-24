#include "posterize_effect.h"

#include <algorithm>
#include "Engine/engine_service_locator.h"

void PosterizeEffect::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    m_context = context;
    m_constantBuffer = SHADER_REPOSITORY->GenerateConstantBufferResource(
        "PosterizeBuffer", 0, sizeof(ConstantBufferData), false, true, ConstantBufferUsage::Dynamic);

    ShaderProgramResource resource;
    resource.name = "PosterizeEffect";
    resource.baseShader = SHADER_REPOSITORY->GetShaderProgramResource(ShaderBase::FullScreen);
    resource.overridePixelShader = SHADER_REPOSITORY->GetPixelShaderResource("posterize_ps.cso");
    resource.additionalConstantBuffers.push_back(m_constantBuffer);
    m_shader = SHADER_REPOSITORY->GenerateShaderProgramResource(resource);
}

void PosterizeEffect::Process(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV,
    int levels, float strength)
{
    if (!m_context || !inputSRV || !outputRTV || !m_constantBuffer || !m_shader) return;

    ConstantBufferData data = {};
    data.levels = (std::max)(levels, 2);
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

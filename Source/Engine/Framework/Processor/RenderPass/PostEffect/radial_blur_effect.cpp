#include "radial_blur_effect.h"

#include <algorithm>
#include "Engine/engine_service_locator.h"

#define SHADER_REPOSITORY EngineServiceLocator::GetShaderRepository()

void RadialBlurEffect::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    m_context = context;

    m_constantBuffer = SHADER_REPOSITORY->GenerateConstantBufferResource(
        "RadialBlurBuffer", 0, sizeof(ConstantBufferData), false, true, ConstantBufferUsage::Dynamic);

    ShaderProgramResource shaderResource;
    shaderResource.name = "RadialBlurEffect";
    shaderResource.baseShader = SHADER_REPOSITORY->GetShaderProgramResource(ShaderBase::FullScreen);
    shaderResource.overridePixelShader = SHADER_REPOSITORY->GetPixelShaderResource("radial_blur_ps.cso");
    shaderResource.additionalConstantBuffers.push_back(m_constantBuffer);
    m_shader = SHADER_REPOSITORY->GenerateShaderProgramResource(shaderResource);
}

void RadialBlurEffect::Process(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV,
    int sampleCount, float strength)
{
    if (!m_context || !inputSRV || !outputRTV || !m_constantBuffer) return;

    ConstantBufferData parameters = {};
    parameters.sampleCount = (std::max)(sampleCount, 1);
    parameters.strength = strength;

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (SUCCEEDED(m_context->Map(m_constantBuffer->buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        *static_cast<ConstantBufferData*>(mapped.pData) = parameters;
        m_context->Unmap(m_constantBuffer->buffer.Get(), 0);
    }

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

// gaussian.cpp
#include "gaussian.h"

#include "Engine/engine_service_locator.h"

void GaussianEffect::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    m_context = context;
    m_constantBuffer = SHADER_REPOSITORY->GenerateConstantBufferResource(
        "GaussianBuffer", 0, sizeof(ConstantBufferData), false, true, ConstantBufferUsage::Dynamic);
    ShaderProgramResource resource;
    resource.name = "GaussianEffect";
    resource.baseShader = SHADER_REPOSITORY->GetShaderProgramResource(ShaderBase::FullScreen);
    resource.overridePixelShader = SHADER_REPOSITORY->GetPixelShaderResource("gaussian_ps.cso");
    resource.additionalConstantBuffers.push_back(m_constantBuffer);
    m_shader = SHADER_REPOSITORY->GenerateShaderProgramResource(resource);
}

void GaussianEffect::Process(ID3D11ShaderResourceView* inputSRV,
    ID3D11RenderTargetView* outputRTV, float dispersion, float strength)
{
    if (!m_context || !inputSRV || !outputRTV || !m_constantBuffer || !m_shader) return;
    
    ConstantBufferData data = {};
    data.strength = strength;
    CalculateGaussianWeights(dispersion, data.weights);

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

void GaussianEffect::CalculateGaussianWeights(float dispersion, float* weights)
{
    weights[0] = expf(0.0f);
    float total = weights[0];

    for (int i = 1; i < 8; ++i) {
        float x = static_cast<float>(i);
        weights[i] = expf(-0.5f * (x * x) / dispersion);
        total += 2.0f * weights[i];
    }

    for (int i = 0; i < 8; ++i) {
        weights[i] /= total;
    }

    return;
}

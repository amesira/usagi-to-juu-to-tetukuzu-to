#pragma once

#include "Engine/Device/direct3d.h"
#include <wrl/client.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class ConstantBufferResource;
class ShaderProgramResource;

class BloomEffect {
private:
    struct ConstantBufferData {
        union {
            XMFLOAT4 data[2];
            struct {
                float threshold;
                float padding[3];
            } brightnessExtract;
            struct {
                XMFLOAT2 texelSize;
                XMFLOAT2 direction;
                float blur;
                float padding[3];
            } gaussianBlur;
        };
    };

    enum class DownsampleLevel {
        Half = 0,
        Quarter,
        Eighth,
        Sixteenth,
        Max,
    };

    static constexpr int DownsampleLevelCount = static_cast<int>(DownsampleLevel::Max);

    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;

    ShaderProgramResource* m_brightnessExtractShader = nullptr;
    ShaderProgramResource* m_gaussianBlurShader = nullptr;
    ShaderProgramResource* m_combineShader = nullptr;
    ConstantBufferResource* m_constantBuffer = nullptr;
    ConstantBufferData m_constantBufferData = {};

    ComPtr<ID3D11Texture2D> m_downsampledTexture[DownsampleLevelCount * 2];
    ComPtr<ID3D11RenderTargetView> m_downsampledRTV[DownsampleLevelCount * 2];
    ComPtr<ID3D11ShaderResourceView> m_downsampledSRV[DownsampleLevelCount * 2];
    unsigned int m_downsampledWidth[DownsampleLevelCount] = {};
    unsigned int m_downsampledHeight[DownsampleLevelCount] = {};

public:
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    void Process(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV);

private:
    void UpdateConstantBuffer();
    void UnbindShaderResources(UINT startSlot, UINT count);
};

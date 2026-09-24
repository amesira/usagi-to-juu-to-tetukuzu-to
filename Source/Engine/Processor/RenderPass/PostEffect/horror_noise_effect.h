#pragma once
#include "Engine/Device/direct3d.h"

class ConstantBufferResource;
class ShaderProgramResource;

class HorrorNoiseEffect {
private:
    struct ConstantBufferData {
        float noiseMin;
        float noiseMax;
        float contrastPow;
        float strength;
        float time;
        float padding[3];
    };

    ID3D11DeviceContext* m_context = nullptr;
    ShaderProgramResource* m_shader = nullptr;
    ConstantBufferResource* m_constantBuffer = nullptr;

public:
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    void Process(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV,
        float noiseMin, float noiseMax, float contrastPow, float strength, float time);
};

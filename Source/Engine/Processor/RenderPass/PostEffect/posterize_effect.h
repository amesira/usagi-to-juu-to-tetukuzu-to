#pragma once
#include "Engine/Device/direct3d.h"

class ConstantBufferResource;
class ShaderProgramResource;

class PosterizeEffect {
private:
    struct ConstantBufferData {
        int levels;
        float strength;
        float padding[2];
    };

    ID3D11DeviceContext* m_context = nullptr;
    ShaderProgramResource* m_shader = nullptr;
    ConstantBufferResource* m_constantBuffer = nullptr;

public:
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    void Process(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV,
        int levels, float strength);
};

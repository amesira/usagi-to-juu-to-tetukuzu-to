#pragma once

#include "Engine/Device/direct3d.h"

using namespace DirectX;

class ConstantBufferResource;
class ShaderProgramResource;

class MonoMaskEffect {
private:
    struct ConstantBufferData {
        XMFLOAT4 monoColor;
        float strength;
        float padding[3];
    };

    ID3D11DeviceContext* m_context = nullptr;
    ShaderProgramResource* m_shader = nullptr;
    ConstantBufferResource* m_constantBuffer = nullptr;

public:
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    void Process(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV,
        ID3D11ShaderResourceView* maskSRV, const XMFLOAT4& monoColor, float strength);
};

#pragma once
#include "Engine/Device/direct3d.h"

struct CustomPostEffectState {
    struct RadialBlur {
        int sampleCount = 4;
        float strength = 0.0f;
    } radialBlur;

    struct MonoMask {
        DirectX::XMFLOAT4 monoColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        float strength = 0.0f;
    } monoMask;

    ID3D11ShaderResourceView* monoMaskTextureSRV = nullptr;

    void Reset() {
        radialBlur = {};
        monoMask = {};
        monoMaskTextureSRV = nullptr;
    }
};

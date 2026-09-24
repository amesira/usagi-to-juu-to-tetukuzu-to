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

    struct Mosaic {
        float mosaicSize = 8.0f;
        float strength = 0.0f;
    } mosaic;

    struct ChromaticAberration {
        float redShiftPixels = 4.0f;
        float blueShiftPixels = -4.0f;
        float strength = 0.0f;
    } chromaticAberration;

    struct Posterize {
        int levels = 4;
        float strength = 0.0f;
    } posterize;

    struct HorrorNoise {
        float noiseMin = -0.15f;
        float noiseMax = 0.15f;
        float contrastPow = 1.5f;
        float strength = 0.0f;
        float time = 0.0f;
    } horrorNoise;

    void Reset() {
        radialBlur = {};
        monoMask = {};
        monoMaskTextureSRV = nullptr;
        mosaic = {};
        chromaticAberration = {};
        posterize = {};
        horrorNoise = {};
    }
};

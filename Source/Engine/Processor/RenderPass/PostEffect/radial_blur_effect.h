//---------------------------------------------------
// File  ：_/RenderPass/PostEffect/radial_blur_effect.h
// Date  ：2026/08/18
// Author：Miu Kitamura
// 
// ・PostEffectのラディアルブラーエフェクトを実装する
//---------------------------------------------------
#pragma once
#include "Engine/Device/direct3d.h"

class ConstantBufferResource;
class ShaderProgramResource;

class RadialBlurEffect {
private:
    /// @brief ラディアルブラーの定数バッファデータ構造体
    struct ConstantBufferData {
        int sampleCount;
        float strength;
        float padding[2];
    };

    ID3D11DeviceContext* m_context = nullptr;

    // === ラディアルブラー用のシェーダーと定数バッファ ===

    ShaderProgramResource* m_radialBlurShader = nullptr;
    ConstantBufferResource* m_constantBuffer = nullptr;

public:
    /// @brief ラディアルブラーエフェクトを初期化する
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);

    /// @brief ラディアルブラーエフェクトを破棄する
    /// @param sampleCount サンプリング数
    /// @param strength ブラーの強さ
    void Process(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV,
        int sampleCount, float strength);

};

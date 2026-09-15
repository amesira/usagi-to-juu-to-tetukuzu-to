//---------------------------------------------------
// File  ：_/RenderPass/PostEffect/mono_mask_effect.h
// Date  ：2026/08/18
// Author：Miu Kitamura
// 
// ・PostEffectのモノクロマスクエフェクトを実装する
//---------------------------------------------------
#pragma once
#include "Engine/Device/direct3d.h"
using namespace DirectX;

class ConstantBufferResource;
class ShaderProgramResource;

class MonoMaskEffect {
private:
    /// @brief モノクロマスクの定数バッファデータ構造体
    struct ConstantBufferData {
        XMFLOAT4 monoColor;
        float strength;
        float padding[3];
    };

    ID3D11DeviceContext* m_context = nullptr;

    // === モノクロマスク用のシェーダーと定数バッファ ===

    ShaderProgramResource* m_monoMaskShader = nullptr;
    ConstantBufferResource* m_constantBuffer = nullptr;

public:
    /// @brief モノクロマスクエフェクトを初期化する
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);

    /// @brief モノクロマスクエフェクトを破棄する
    /// @param monoColor マスク適用時のモノクロカラー
    /// @param strength マスクの強さ（0.0f～1.0f）
    void Process(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV,
        ID3D11ShaderResourceView* maskSRV, const XMFLOAT4& monoColor, float strength);
};

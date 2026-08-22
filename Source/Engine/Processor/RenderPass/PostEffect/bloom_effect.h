//---------------------------------------------------
// File  ：_/RenderPass/PostEffect/bloom_effect.h
// Date  ：2026/08/17
// Author：Miu Kitamura
// 
// ・PostEffectのブルームエフェクトを実装する
//---------------------------------------------------
#pragma once
#include "Engine/Device/direct3d.h"
#include <wrl/client.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class ConstantBufferResource;
class ShaderProgramResource;

class BloomEffect {
private:
    /// @brief ブルームエフェクトの定数バッファデータ構造体
    struct ConstantBufferData {
        union {
            XMFLOAT4 data[2];
            // 明るさ抽出用のパラメータ
            struct {
                float threshold;
                float padding[3];
            } brightnessExtract;
            // ガウシアンブラー用のパラメータ
            struct {
                XMFLOAT2 texelSize;
                XMFLOAT2 direction;
                float blur;
                float padding[3];
            } gaussianBlur;
        };
    };

    /// @brief ダウンサンプリングレベルの列挙型
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
    /// @brief ブルームエフェクトを初期化する
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    /// @brief ブルームエフェクトを破棄する
    void Process(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV);

private:
    /// @brief ブルームエフェクトの定数バッファを更新する
    void UpdateBrightnessConstantBuffer(float threshold);
    void UpdateGaussianBlurConstantBuffer(const XMFLOAT2& texelSize, const XMFLOAT2& direction, float blur);

    /// @brief シェーダーリソースビューを解除する
    void UnbindShaderResources(UINT startSlot, UINT count);

};

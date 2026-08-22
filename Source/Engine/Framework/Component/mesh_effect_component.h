//---------------------------------------------------
// File  ：_/Component/mesh_effect_component.h
// Date  ：2026/08/21
// Author：Miu Kitamura
// 
// ・MeshEffectを扱うComponent
//---------------------------------------------------
#pragma once
#include "Engine/Core/component.h"

#include "Engine/Graphics/texture_resource.h"
#include "Engine/Graphics/model_resource.h"
#include <iostream>

#include "Engine/Device/direct3d.h"
using namespace DirectX;

#include "Engine/Asset/MeshEffectAsset/mesh_effect_data.h"
#include "Engine/Asset/MeshEffectAsset/mesh_effect_asset.h"

namespace MeshEffectRenderData{
    /// @brief MeshEffectComponentのPSへ転送するためのバッファ構造体
    struct MeshEffectBuffer 
    {
        // GradientModule::colorを評価した結果
        XMFLOAT4 effectColor;

        // Flipbookを含めて計算した最終UV領域
        // xy: 左上、zw: 幅・高さ
        XMFLOAT4 frameUVRect;

        // ScrollModule
        XMFLOAT2 uvTiling;
        XMFLOAT2 uvOffset;

        // UV Wave
        XMFLOAT2 uvWaveDirection;
        float  uvWaveAmplitude;
        float  uvWaveFrequency;

        float  uvWaveSpeed;
        float  effectTime;
        float  alphaCutoff;

        float padding;
    };

    /// @brief MeshEffectComponentの評価済み状態を保持する構造体
    struct MeshEffectEvaluatedState
    {
        bool visible = false;

        // Transformカーブの評価結果
        DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
        DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };

        // TransformComponentと合成する前のエフェクト変形
        DirectX::XMMATRIX localEffectMatrix = DirectX::XMMatrixIdentity();

        // PSへ転送する評価済み値
        MeshEffectBuffer buffer = {};

        MeshEffectData::BlendMode blendMode = MeshEffectData::BlendMode::AlphaBlend;
    };
}

class MeshEffectComponent : public Component {
private:
    // アセットの参照と実データ
    class MeshEffectAsset* m_asset = nullptr;
    MeshEffectDesc m_desc = {};

    // リソース参照
    ModelResource* m_modelResource = nullptr;
    TextureResource* m_textureResource = nullptr;

    bool m_isPlaying = false;
    float m_time;

    MeshEffectRenderData::MeshEffectEvaluatedState m_evaluatedState;

public:
    // === アセットの設定・取得 ===
    void SetAsset(MeshEffectAsset* asset) { m_asset = asset; m_desc = asset->GetDesc(); }
    MeshEffectAsset* GetAsset() { return m_asset; }
    const MeshEffectAsset* GetAsset() const { return m_asset; }

    // == リソースの設定・取得 ===
    void SetModelResource(ModelResource* modelResource) { m_modelResource = modelResource; }
    ModelResource* GetModelResource() { return m_modelResource; }
    void SetTextureResource(TextureResource* textureResource) { m_textureResource = textureResource; }
    TextureResource* GetTextureResource() { return m_textureResource; }

    // === 再生制御 ===
    void Play() { m_isPlaying = true; m_time = 0.0f; }
    void Stop() { m_isPlaying = false; m_time = 0.0f; }
    void Pause() { m_isPlaying = false; }

    bool IsPlaying() const { return m_isPlaying; }
    void SetTime(float time) { m_time = time; }
    float GetTime() const { return m_time; }

    MeshEffectRenderData::MeshEffectEvaluatedState& EvaluatedState() { return m_evaluatedState; }

#pragma region 各モジュールのアクセス
    MeshEffectData::MainModule& Main() { return m_desc.mainModule; }
    MeshEffectData::TransformModule& Transform() { return m_desc.transformModule; }
    MeshEffectData::FlipbookModule& Flipbook() { return m_desc.flipbookModule; }
    MeshEffectData::ScrollModule& Scroll() { return m_desc.scrollModule; }
    MeshEffectData::WaveModule& Wave() { return m_desc.waveModule; }
    MeshEffectData::GradientModule& Gradient() { return m_desc.gradientModule; }
    MeshEffectData::RendererModule& Renderer() { return m_desc.rendererModule; }
#pragma endregion

};
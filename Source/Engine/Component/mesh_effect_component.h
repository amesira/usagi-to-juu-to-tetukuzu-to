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
        XMFLOAT4 effectColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        XMFLOAT4 fresnelColor = { 1.0f, 1.0f, 1.0f, 1.0f };

        // Flipbookを含めて計算した最終UV領域
        // xy: 左上、zw: 幅・高さ
        XMFLOAT4 frameUVRect = { 0.0f, 0.0f, 1.0f, 1.0f };

        // ScrollModule
        XMFLOAT2 uvTiling = { 1.0f, 1.0f };
        XMFLOAT2 uvOffset = { 0.0f, 0.0f };

        // UV Wave
        XMFLOAT2 uvWaveDirection = { 0.0f, 0.0f };
        float  uvWaveAmplitude = 0.0f;
        float  uvWaveFrequency = 0.0f;

        float  uvWaveSpeed = 0.0f;
        // Fresnal
        int    useFresnel = 0;
        float  fresnelThreshold = 0.0f;
        float  fresnelIntensity = 1.0f;
        
        float  effectTime = 0.0f;
        float  alphaCutoff = 0.0f;
        int    useWorldProjection = 0;
        float rendererIntensity = 1.0f;

        // Gradient Over UV
        XMFLOAT4 gradientStartColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        XMFLOAT4 gradientEndColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        XMFLOAT2 gradientDirection = { 0.0f, 1.0f };
        float gradientStartPosition = 0.0f;
        float gradientEndPosition = 1.0f;
    };
    static_assert(sizeof(MeshEffectBuffer) % 16 == 0);

    /// @brief MeshEffectComponentの評価済み状態を保持する構造体
    struct MeshEffectEvaluatedState
    {
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
    bool m_hasPlayed = false;
    float m_time = 0.0f;        // 経過時間（リセットは基本行なわない）
    float m_loopTime = 0.0f;    // ループ時間（durationを超えた場合にリセットされる）

    MeshEffectRenderData::MeshEffectEvaluatedState m_evaluatedState;

public:
    // === アセットの設定・取得 ===
    void SetAsset(MeshEffectAsset* asset) {
        m_asset = asset;
        m_desc = asset ? asset->GetDesc() : MeshEffectDesc{};
        m_modelResource = nullptr;
        m_textureResource = nullptr;
    }
    MeshEffectAsset* GetAsset() { return m_asset; }
    const MeshEffectAsset* GetAsset() const { return m_asset; }

    // == リソースの設定・取得 ===
    void SetModelResource(ModelResource* modelResource) { m_modelResource = modelResource; }
    ModelResource* GetModelResource() { return m_modelResource; }
    ModelResource* GetModelResource() const { return m_modelResource; }
    void SetTextureResource(TextureResource* textureResource) { m_textureResource = textureResource; }
    TextureResource* GetTextureResource() { return m_textureResource; }
    TextureResource* GetTextureResource() const { return m_textureResource; }

    // === 再生制御 ===
    void Play() { m_isPlaying = true; m_hasPlayed = true; m_time = 0.0f; m_loopTime = 0.0f; }
    void Stop() { m_isPlaying = false; m_time = 0.0f; m_loopTime = 0.0f; }
    void Pause() { m_isPlaying = false; }

    bool IsPlaying() const { return m_isPlaying; }
    bool HasPlayed() const { return m_hasPlayed; }
    void SetTime(float time) { m_time = time; }
    float GetTime() const { return m_time; }
    void SetLoopTime(float loopTime) { m_loopTime = loopTime; }
    float GetLoopTime() const { return m_loopTime; }

    MeshEffectRenderData::MeshEffectEvaluatedState& EvaluatedState() { return m_evaluatedState; }
    const MeshEffectRenderData::MeshEffectEvaluatedState& EvaluatedState() const { return m_evaluatedState; }

    MeshEffectDesc& GetDesc() { return m_desc; }
    const MeshEffectDesc& GetDesc() const { return m_desc; }

#pragma region 各モジュールのアクセス
    MeshEffectData::MainModule& Main() { return m_desc.mainModule; }
    MeshEffectData::TransformModule& Transform() { return m_desc.transformModule; }
    MeshEffectData::FlipbookModule& Flipbook() { return m_desc.flipbookModule; }
    MeshEffectData::ScrollModule& Scroll() { return m_desc.scrollModule; }
    MeshEffectData::WaveModule& Wave() { return m_desc.waveModule; }
    MeshEffectData::GradientModule& Gradient() { return m_desc.gradientModule; }
    MeshEffectData::FresnelModule& Fresnel() { return m_desc.fresnelModule; }
    MeshEffectData::RendererModule& Renderer() { return m_desc.rendererModule; }
    const MeshEffectData::RendererModule& Renderer() const { return m_desc.rendererModule; }
#pragma endregion

};

//---------------------------------------------------
// File  ：_/Asset/MeshEffectAsset/mesh_effect_data.h
// Date  ：2026/08/21
// Author：Miu Kitamura
// 
// ・MeshEffectのデータクラス
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>
#include <string>
using namespace DirectX;

#include "Utility/mi_curve.h"

namespace MeshEffectData
{
    enum class TimeMode {
        Duration,
        Speed,
    };
    enum class BillboardMode {
        None,
        View,
        Horizontal,
    };
    enum class BlendMode {
        AlphaBlend,
        Additive,
    };

    enum class TextureMappingMode {
        MeshUV,
        WorldProjection
    };
    enum class FlipbookPlaybackMode {
        Once,
        Loop,
        PingPong,
    };
    enum class WaveType {
        Vertex, // 未実装
        UV,
    };

    /// @brief MainModuleの定義（エフェクトシステムの基本設定を行うモジュール）
    struct MainModule {
        float duration = 1.0f;
        bool loop = true;
        bool playOnAwake = true;

        float simulationSpeed = 1.0f;
    };

    /// @brief TransformModuleの定義（サイズ変化や回転などの変換を行うモジュール）
    struct TransformModule {
        bool enabled = true;

        MiCurve::Float3Curve scaleOverDuration = { { {0.0f, {1.0f, 1.0f, 1.0f}}, {1.0f, {1.0f, 1.0f, 1.0f}} } };
        MiCurve::Float3Curve rotationOverDuration = { { {0.0f, {0.0f, 0.0f, 0.0f}}, {1.0f, {0.0f, 0.0f, 0.0f}} } };
    };

    /// @brief FlipbookModuleの定義（フリップブックアニメーションを行うモジュール）
    struct FlipbookModule {
        bool enabled = false;

        int tileX = 1;
        int tileY = 1;
        int startFrame = 0;
        int frameCount = 1;

        TimeMode timeMode = TimeMode::Duration; // アニメーションの時間の種類
        float framePerSecond = 30.0f;           // 1秒あたりのフレーム数 : TimeMode::Speedの場合に使用

        FlipbookPlaybackMode playbackMode = FlipbookPlaybackMode::Loop; // 再生モード
    };

    /// @brief ScrollModuleの定義（テクスチャのスクロールを行うモジュール）
    struct ScrollModule {
        bool enabled = false;

        XMFLOAT2 tiling = { 1.0f, 1.0f };
        XMFLOAT2 offset = { 0.0f, 0.0f };

        XMFLOAT2 scrollSpeed = { 0.0f, 0.0f };
    };

    /// @brief WaveModuleの定義（波の変形を行うモジュール）
    struct WaveModule {
        bool enabled = false;

        WaveType type = WaveType::Vertex;
        XMFLOAT2 direction = { 0.0f, 1.0f };

        MiCurve::FloatCurve amplitudeOverDuration = {}; // 波の振幅の変化
        float frequency = 1.0f; // 波の周波数
        float speed = 1.0f;     // 波の速度
    };

    /// @brief GradientModuleの定義（色の変化を行うモジュール）
    struct GradientModule {
        bool enabled = false;

        MiCurve::Float4Curve gradientOverDuration = { { {0.0f, {1.0f, 1.0f, 1.0f, 1.0f}}, {1.0f, {1.0f, 1.0f, 1.0f, 1.0f}} } };
        MiCurve::Float4Curve gradientOverUV = { { {0.0f, {1.0f, 1.0f, 1.0f, 1.0f}}, {1.0f, {1.0f, 1.0f, 1.0f, 1.0f}} } }; // 未実装
    };

    struct FresnelModule {
        bool enabled = false;

        XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
        float threshold = 0.0f;
        float intensity = 1.0f;
    };

    /// @brief RendererModuleの定義（メッシュの描画設定を行うモジュール）
    struct RendererModule {
        std::string modelPath;
        std::string texturePath;

        TextureMappingMode textureMappingMode = TextureMappingMode::MeshUV;
        XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
        XMFLOAT4 uvRect = { 0.0f, 0.0f, 1.0f, 1.0f };

        BillboardMode billboardMode = BillboardMode::None;
        BlendMode blendMode = BlendMode::AlphaBlend;
    };
}

/// @brief MeshEffectの設定をまとめた構造体
struct MeshEffectDesc {
    MeshEffectData::MainModule mainModule;
    MeshEffectData::TransformModule transformModule;
    MeshEffectData::FlipbookModule flipbookModule;
    MeshEffectData::ScrollModule scrollModule;
    MeshEffectData::WaveModule waveModule;
    MeshEffectData::GradientModule gradientModule;
    MeshEffectData::FresnelModule fresnelModule;
    MeshEffectData::RendererModule rendererModule;
};

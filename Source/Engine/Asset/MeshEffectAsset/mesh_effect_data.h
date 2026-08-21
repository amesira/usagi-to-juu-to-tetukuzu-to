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
    // === タイプ定義 ===
    enum TimeMode {
        Lifetime,
        Speed,
    };
    enum BillboardMode {
        View,
        Horizontal,
    };
    enum BlendMode {
        AlphaBlend,
        Additive,
    };
    enum class FlipbookPlaybackMode {
        Once,
        Loop,
        PingPong,
    };
    enum class WaveType {
        Vertex,
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
        MiCurve::Float3Curve scaleOverLifetime = MiCurve::Float3Curve(1.0f);
        MiCurve::Float3Curve rotationOverLifetime = MiCurve::Float3Curve(0.0f);
    };

    /// @brief FlipbookModuleの定義（フリップブックアニメーションを行うモジュール）
    struct FlipbookModule {
        bool enabled = false;
        int tileX = 1;        // テクスチャの横方向の分割数
        int tileY = 1;        // テクスチャの縦方向の分割数p
        int startFrame = 0;   // アニメーションの開始フレーム
        int frameCount = 1;   // アニメーションの総フレーム数

        TimeMode timeMode = TimeMode::Lifetime; // アニメーションの時間の種類
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
        WaveType type = WaveType::Vertex;   // 波の種類
        XMFLOAT3 direction = { 0.0f, 1.0f, 0.0f };      // 波の方向
        MiCurve::FloatCurve amplitudeOverLifetime = {}; // 波の振幅の変化
        float frequency = 1.0f; // 波の周波数
        float speed = 1.0f;     // 波の速度
    };

    /// @brief GradientModuleの定義（色の変化を行うモジュール）
    struct GradientModule {
        bool enabled = false;
        MiCurve::ColorCurve color = {};
    };

    /// @brief RendererModuleの定義（メッシュの描画設定を行うモジュール）
    struct RendererModule {
        std::string meshPath;
        std::string texturePath;

        XMFLOAT4 uvRect = { 0.0f, 0.0f, 1.0f, 1.0f };       // テクスチャのUV矩形

        BillboardMode billboardMode = BillboardMode::View;  // ビルボードの種類
        BlendMode blendMode = BlendMode::AlphaBlend;        // ブレンドモード
    };
}

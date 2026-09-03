//---------------------------------------------------
// File  ：_/Asset/EnvironmentAsset/environment_lighting_data.h
// Date  ：2026/09/04
// Author：Miu Kitamura
// 
// ・環境ライト設定のデータ構造を定義するヘッダファイル
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>
using namespace DirectX;

// DirectionalLightの設定構造体
struct DirectionalLightSettings {
    bool    enabled = true;
    float   intensity = 1.0f;
    XMFLOAT3 direction = { -0.5f, -1.0f, -0.5f };
    XMFLOAT3 color = { 1.0f, 1.0f, 1.0f };
    XMFLOAT3 ambientColor = { 0.1f, 0.1f, 0.1f }; // 環境光の色
};

// リムライト設定構造体
struct RimLightSettings {
    bool    enabled = true;
    float   intensity = 1.0f;
    float   threshold = 0.7f;              // リムライトの閾値（法線と視線の角度）
    XMFLOAT3 color = { 1.0f, 1.0f, 1.0f }; // リムライトの色
};

// 半球ライト設定構造体
struct HemisphereLightSettings {
    bool    enabled = true;
    float   intensity = 0.5f;
    XMFLOAT3 skyColor = { 0.5f, 0.5f, 0.5f };   // 空の色
    XMFLOAT3 groundColor = { 0.2f, 0.2f, 0.2f };// 地面の色
};

// 6面で考えてライトを当ててみる
struct CubicColorLightSettings {
    bool enabled = false;
    float intensity = 1.0f;
    XMFLOAT3 upColor = { 1.0f, 1.0f, 1.0f };
    XMFLOAT3 downColor = { 1.0f, 1.0f, 1.0f };
    XMFLOAT3 leftColor = { 1.0f, 1.0f, 1.0f };
    XMFLOAT3 rightColor = { 1.0f, 1.0f, 1.0f };
    XMFLOAT3 frontColor = { 1.0f, 1.0f, 1.0f };
    XMFLOAT3 backColor = { 1.0f, 1.0f, 1.0f };
};

/// @brief 環境ライト設定のデータ構造
struct EnvironmentLightingData {
    DirectionalLightSettings directionalLight;
    RimLightSettings rimLight;
    HemisphereLightSettings hemisphereLight;
    CubicColorLightSettings cubicColorLight;
};

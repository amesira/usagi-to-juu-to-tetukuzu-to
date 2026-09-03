//---------------------------------------------------
// File  ：_/Asset/EnvironmentAsset/environment_asset.h
// Date  ：2026/09/04
// Author：Miu Kitamura
// 
// ・環境設定のアセットクラス
//---------------------------------------------------
#pragma once
#include "Engine/Asset/i_asset.h"
#include "environment_lighting_data.h"
#include "environment_post_process_data.h"

class EnvironmentLightingAsset : public IAsset {
private:
    EnvironmentLightingData m_lightingData;
public:
    const EnvironmentLightingData& GetData() const { return m_lightingData; }
    EnvironmentLightingData& GetData() { return m_lightingData; }
};

class EnvironmentPostProcessAsset : public IAsset {
private:
    EnvironmentPostProcessData m_postProcessData;
public:
    const EnvironmentPostProcessData& GetData() const { return m_postProcessData; }
    EnvironmentPostProcessData& GetData() { return m_postProcessData; }
};
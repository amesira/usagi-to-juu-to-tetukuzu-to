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

struct EnvironmentData {
    EnvironmentLightingData lighting;
    EnvironmentPostProcessData postProcess;
};

class EnvironmentAsset : public IAsset {
private:
    EnvironmentData m_data;

public:
    const EnvironmentData& GetData() const { return m_data; }
    EnvironmentData& GetData() { return m_data; }
};

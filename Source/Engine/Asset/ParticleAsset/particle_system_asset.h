//---------------------------------------------------
// particle_system_asset.h
// ・ParticleSystemのアセットを表すクラスの定義
// 
// Author：Miu Kitamura
// Date  ：2026/06/28
//---------------------------------------------------
#pragma once
#include "Engine/Asset/i_asset.h"
#include "particle_system_data.h"

// 各モジュールのデータ
struct ParticleSystemDesc {
    ParticleSystemData::MainModule mainModule;
    ParticleSystemData::EmissionModule emissionModule;
    ParticleSystemData::ShapeModule shapeModule;
    ParticleSystemData::SizeOverLifetimeModule sizeOverLifetimeModule;
    ParticleSystemData::TextureSheetAnimation textureSheetAnimation;
    ParticleSystemData::RendererModule rendererModule;
};

class ParticleSystemAsset : public IAsset {
private:
    ParticleSystemDesc m_desc = {};

public:
    // Descの取得・コピー
    const ParticleSystemDesc& GetDesc() const { return m_desc; }
    ParticleSystemDesc CopyDesc() const { return m_desc; }

    void SetDesc(const ParticleSystemDesc& desc) { m_desc = desc; }
};
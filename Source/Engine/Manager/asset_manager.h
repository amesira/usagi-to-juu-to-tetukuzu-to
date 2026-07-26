//---------------------------------------------------
// File  ：Engine/Manager/asset_manager.h
// Date  ：2026/07/26
// Author：Miu Kitamura
// 
// ・アセット管理を行うクラス
//---------------------------------------------------
#pragma once
#include "Engine/Asset/ParticleAsset/particle_system_asset_loader.h"

class AssetManager {
private:
    // ParticleSystemAssetLoaderのインスタンス
    ParticleSystemAssetLoader m_particleAssetLoader;

public:
    /// @brief AssetManagerを初期化する
    void Initialize();
    /// @brief AssetManagerを終了する
    void Finalize();

    /// @brief ParticleSystemAssetLoaderを取得
    ParticleSystemAssetLoader* GetParticleAssetLoader() { return &m_particleAssetLoader; }

};
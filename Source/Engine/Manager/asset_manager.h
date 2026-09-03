//---------------------------------------------------
// File  ：Engine/Manager/asset_manager.h
// Date  ：2026/07/26
// Author：Miu Kitamura
// 
// ・アセット管理を行うクラス
//---------------------------------------------------
#pragma once
#include "Engine/Asset/ParticleAsset/particle_system_asset_loader.h"
#include "Engine/Asset/DataAsset/data_asset_loader.h"
#include "Engine/Asset/MeshEffectAsset/mesh_effect_asset_loader.h"
#include "Engine/Asset/EnvironmentAsset/environment_asset_loader.h"

class AssetManager {
private:
    // DataAssetLoaderのインスタンス
    DataAssetLoader m_dataAssetLoader;
    // ParticleSystemAssetLoaderのインスタンス
    ParticleSystemAssetLoader m_particleAssetLoader;
    // MeshEffectAssetLoaderのインスタンス
    MeshEffectAssetLoader m_meshEffectAssetLoader;
    EnvironmentAssetLoader m_environmentAssetLoader;

public:
    /// @brief AssetManagerを初期化する
    void Initialize();
    /// @brief AssetManagerを終了する
    void Finalize();

    /// @brief DataAssetLoaderを取得
    DataAssetLoader* DataAssetLoader() { return &m_dataAssetLoader; }
    /// @brief ParticleSystemAssetLoaderを取得
    ParticleSystemAssetLoader* ParticleAssetLoader() { return &m_particleAssetLoader; }
    /// @brief MeshEffectAssetLoaderを取得
    MeshEffectAssetLoader* MeshEffectAssetLoader() { return &m_meshEffectAssetLoader; }
    EnvironmentAssetLoader* EnvironmentAssetLoader() { return &m_environmentAssetLoader; }

};

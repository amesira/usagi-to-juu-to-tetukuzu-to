//===================================================
// File  ：Engine/Manager/asset_manager.cpp
// Date  ：2026/07/26
// Author：Miu Kitamura
// 
// ・アセット管理を行うクラス
//===================================================
#include "asset_manager.h"

/// @brief AssetManagerの初期化
void AssetManager::Initialize()
{
    m_dataAssetLoader.Initialize();
    m_particleAssetLoader.Initialize();
    m_meshEffectAssetLoader.Initialize();
    m_environmentAssetLoader.Initialize();
}

/// @brief AssetManagerの終了処理
void AssetManager::Finalize()
{
    m_dataAssetLoader.Finalize();
    m_particleAssetLoader.Finalize();
    m_meshEffectAssetLoader.Finalize();
    m_environmentAssetLoader.Finalize();
}

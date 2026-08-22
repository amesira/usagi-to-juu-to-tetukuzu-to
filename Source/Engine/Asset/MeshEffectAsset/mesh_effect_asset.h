//---------------------------------------------------
// File  ：_/Asset/MeshEffectAsset/mesh_effect_asset.h
// Date  ：2026/08/21
// Author：Miu Kitamura
// 
// ・MeshEffectのアセットクラス
//---------------------------------------------------
#pragma once
#include "Engine/Asset/i_asset.h"
#include "mesh_effect_data.h"

/// @brief MeshEffectのアセットクラス
class MeshEffectAsset : public IAsset {
private:
    MeshEffectDesc m_desc = {};

public:
    const MeshEffectDesc& GetDesc() const { return m_desc; }
    MeshEffectDesc& GetDesc() { return m_desc; }
};

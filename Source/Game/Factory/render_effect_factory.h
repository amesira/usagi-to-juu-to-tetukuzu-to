//---------------------------------------------------
// render_effect_factory.h
// 
// Author：Miu Kitamura
// Date  ：2026/04/14
//---------------------------------------------------
#ifndef RENDER_EFFECT_FACTORY_H
#define RENDER_EFFECT_FACTORY_H
#include <DirectXMath.h>
using namespace DirectX;
#include <string>
#include <filesystem>
#include "Game/PresBehavior/effect_handle.h"

class GameObject;
class IScene;
class TransformComponent;

namespace RenderEffectFactory {
    // デカールエフェクト生成
    GameObject* CreateDecalEffect(IScene* scene, const XMFLOAT3& position, const std::wstring& decalTexturePath);
    
    EffectHandle CreateParticleEffect(
        IScene* scene, 
        const XMFLOAT3& position, 
        const std::filesystem::path& assetPath);
    EffectHandle CreateMeshEffect(
        IScene* scene,
        const XMFLOAT3& position,
        const std::filesystem::path& assetPath);

    /// @brief Transformへ追従するParticleEffectを生成する
    EffectHandle CreateAttachedParticleEffect(
        IScene* scene,
        TransformComponent* target,
        const std::filesystem::path& assetPath,
        const XMFLOAT3& offset);

    /// @brief Transformへ追従するMeshEffectを生成する
    EffectHandle CreateAttachedMeshEffect(
        IScene* scene,
        TransformComponent* target,
        const std::filesystem::path& assetPath,
        const XMFLOAT3& offset);
};

#endif // !RENDER_EFFECT_FACTORY_H

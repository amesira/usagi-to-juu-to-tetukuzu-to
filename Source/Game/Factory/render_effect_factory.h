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
#include "Game/PresBehavior/attached_effect_handle.h"
#include "Game/PresBehavior/effect_transform.h"

class GameObject;
class IScene;
class TransformComponent;
class ParticleSystemComponent;

struct EffectAttachmentDesc {
    TransformComponent* target = nullptr;
    EffectTransform localTransform{};
    bool considerTargetRotation = true;
    bool considerTargetScaling = false;
};

namespace RenderEffectFactory {
    // 既存のComponentへパーティクル設定とテクスチャを適用する
    bool ApplyParticleAsset(
        ParticleSystemComponent* particleSystem,
        const std::filesystem::path& assetPath,
        const std::wstring* textureOverride = nullptr);

    // デカールエフェクト生成
    GameObject* CreateDecalEffect(IScene* scene, const XMFLOAT3& position, const std::wstring& decalTexturePath);
    
    EffectHandle CreateParticleEffect(
        IScene* scene,
        const std::filesystem::path& assetPath,
        const EffectTransform& transform = {});
    EffectHandle CreateMeshEffect(
        IScene* scene,
        const std::filesystem::path& assetPath,
        const EffectTransform& transform = {});

    /// @brief Transformへ追従するParticleEffectを生成する
    AttachedEffectHandle CreateAttachedParticleEffect(
        IScene* scene,
        const std::filesystem::path& assetPath,
        const EffectAttachmentDesc& attachment);

    /// @brief Transformへ追従するMeshEffectを生成する
    AttachedEffectHandle CreateAttachedMeshEffect(
        IScene* scene,
        const std::filesystem::path& assetPath,
        const EffectAttachmentDesc& attachment);
};

#endif // !RENDER_EFFECT_FACTORY_H

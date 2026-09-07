// projectile_factory.h
#pragma once
#include <DirectXMath.h>
#include <filesystem>
#include "Engine/Core/game_object_layer.h"

#include <string>

using namespace DirectX;

class GameObject;
class IScene;
class TextureResource;

namespace ProjectileFactory
{
    struct BulletCreateDesc {
        XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
        XMFLOAT3 velocity = { 0.0f, 0.0f, 1.0f };
        float radius = 0.25f;
        float lifeTime = 3.0f;
        CollisionLayerMask layerMask = COLLISION_LAYER_MASK_ALL;

        const char* modelPath = "asset\\Model\\bullet.fbx";
        const char* materialName = "BulletHologramMaterial";
    };
    struct DamageNumberCreateDesc {
        XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
        float damage = 1.0f;
        XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
        const std::filesystem::path fontPath = "asset/Font/Makinas-4-Square.otf";
    };

    GameObject* CreateBullet(IScene* scene, const BulletCreateDesc& desc);
    GameObject* CreateDamageNumber(IScene* scene, const DamageNumberCreateDesc& desc);
}

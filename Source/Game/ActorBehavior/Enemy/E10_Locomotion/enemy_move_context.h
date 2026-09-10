// enemy_move_context.h
// 2026/09/10
#pragma once
#include <DirectXMath.h>
#include "enemy_move_settings_asset.h"

struct EnemyMoveRuntimeState {
    DirectX::XMFLOAT3 controlVelocity = {};
    DirectX::XMFLOAT3 desiredVelocity = {};
    bool isGrounded = false;
    bool wasGrounded = false;
};

struct EnemyMoveContext {
    class GameObject* owner = nullptr;
    class IScene* scene = nullptr;
    class TransformComponent* transform = nullptr;
    class RigidbodyComponent* rigidbody = nullptr;
    class CapsuleColliderComponent* collider = nullptr;

    const EnemyMoveSettingsAsset* settingsAsset = nullptr;
    EnemyMoveRuntimeState runtimeState;

    const EnemyMoveSettings::Data& settings() const
    {
        static const EnemyMoveSettings::Data defaults;
        return settingsAsset ? settingsAsset->GetData() : defaults;
    }
};

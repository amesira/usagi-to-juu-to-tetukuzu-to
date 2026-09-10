// enemy_move_context.h
// 2026/09/10
#pragma once
#include <DirectXMath.h>
#include "enemy_move_settings_asset.h"

struct EnemyMoveRuntimeState {
    DirectX::XMFLOAT3 controlVelocity = {};
    DirectX::XMFLOAT3 physicsVelocity = {};
    DirectX::XMFLOAT3 desiredPosition = {};
    bool isGrounded = false;
};

struct EnemyMoveContext {
    class GameObject* owner = nullptr;
    class IScene* scene = nullptr;

    class TransformComponent* transform = nullptr;
    class RigidbodyComponent* rigidbody = nullptr;
    class CapsuleColliderComponent* collider = nullptr;

    const EnemyMoveSettingsAsset* settingsAsset = nullptr;
    EnemyMoveRuntimeState runtimeState;

    class EnemyMoveMotor* moveMotor = nullptr;
    class EnemyMoveEffects* moveEffects = nullptr;

    const EnemyMoveSettings::Data& settings() const
    {
        static const EnemyMoveSettings::Data defaults;
        return settingsAsset ? settingsAsset->GetData() : defaults;
    }
};

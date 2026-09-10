#pragma once

#include <DirectXMath.h>

class EnemyBehavior;
class EnemyAIController;
class EnemyLocomotionController;
class EnemyMoveBehavior;
class EnemyConditionMachine;
class EnemyCombatTree;
class EnemyAnimationController;
class EnemyAiAgentSettingsAsset;
class HealthBehavior;
class IScene;
class RigidbodyComponent;
class TransformComponent;

/// @brief 複数の敵機能から参照する、敵個体の実行時情報。
struct EnemyRuntimeState {
    DirectX::XMFLOAT3 controlVelocity = {};
    bool isGrounded = false;

    bool hasCombatTarget = false;
    DirectX::XMFLOAT3 combatTargetPosition = {};
};

/// @brief 敵個体を構成する各機能が共有する参照と状態。
class EnemyContext {
public:
    EnemyBehavior* owner = nullptr;
    IScene* scene = nullptr;
    TransformComponent* transform = nullptr;
    RigidbodyComponent* rigidbody = nullptr;
    HealthBehavior* health = nullptr;
    EnemyAIController* aiWorld = nullptr;

    EnemyLocomotionController* locomotionController = nullptr;
    EnemyMoveBehavior* moveBehavior = nullptr;
    EnemyConditionMachine* conditionMachine = nullptr;
    EnemyCombatTree* combatTree = nullptr;
    EnemyAnimationController* animationController = nullptr;

    const EnemyAiAgentSettingsAsset* aiAgentSettings = nullptr;
    EnemyRuntimeState runtimeState;
};

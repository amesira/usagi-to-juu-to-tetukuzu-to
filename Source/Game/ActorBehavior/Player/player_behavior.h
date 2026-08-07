#ifndef PLAYER_BEHAVIOR_H
#define PLAYER_BEHAVIOR_H

#include "Engine/Framework/Component/behavior_component.h"
#include "Game/ActorBehavior/Player/P00_Core/player_context.h"
#include "Game/ActorBehavior/Player/P00_Core/player_input.h"

class CameraComponent;
class RigidbodyComponent;
class SpriteAnimationComponent;
class SpriteRendererComponent;
class TransformComponent;

class PlayerBehavior : public BehaviorComponent {
private:
    RigidbodyComponent* m_rigidbody = nullptr;
    SpriteRendererComponent* m_spriteRenderer = nullptr;
    SpriteAnimationComponent* m_spriteAnimation = nullptr;

    PlayerContext m_context;
    PlayerInput m_input;

    TransformComponent* m_mainCameraTransform = nullptr;
    CameraComponent* m_mainCamera = nullptr;

public:
    ~PlayerBehavior() = default;

    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

    // Dodge effects remain here until the dodge action is rebuilt.
    void PlayPlayerEffect(PlayerEffectType type);

private:
    PlayerInput UpdateInput();
    void UpdateAnimation(PlayerState state, PlayerCombatState combatState);
};

#endif

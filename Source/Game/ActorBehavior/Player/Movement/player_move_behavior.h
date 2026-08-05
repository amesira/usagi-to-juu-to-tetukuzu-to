//---------------------------------------------------
// player_move_behavior.h
// 
// Author：Miu Kitamura
// Date  ：2026/03/25
//---------------------------------------------------
#ifndef PLAYER_MOVE_BEHAVIOR_H
#define PLAYER_MOVE_BEHAVIOR_H
#include "Engine/Framework/Component/behavior_component.h"
#include "Game/ActorBehavior/Player/player_context.h"
#include "Game/ActorBehavior/Player/player_input.h"

class TransformComponent;
class RigidbodyComponent;
class SpriteRendererComponent;

class CameraComponent;

class PlayerMoveBehavior : public BehaviorComponent {
public:
    // プレイヤーの回転モード
    enum class PlayerRotationMode {
        CameraForward,
        AimForward,
        Locked,
    };

    // プレイヤーの移動リクエスト構造体
    struct PlayerMoveRequest {
        bool canMove = true;
        bool canRotate = true;

        float speedMultiplier = 1.0f;

        PlayerRotationMode rotationMode = PlayerRotationMode::CameraForward;
    };

private:
    TransformComponent* m_transform = nullptr;
    RigidbodyComponent* m_rigidbody = nullptr;
    SpriteRendererComponent* m_spriteRenderer = nullptr;

    float   m_acceleration = 20.0f;
    float   m_moveSpeed = 10.0f;
    float   m_jumpForce = 10.0f;

    float   m_currentAngleY = 0.0f;
    float   m_rotationSpeed = 10.0f;

    // メインカメラの参照
    CameraComponent* m_mainCamera = nullptr;

public:
    PlayerMoveBehavior() = default;
    ~PlayerMoveBehavior() = default;
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

    // 移動更新処理
    void UpdateMove(const PlayerContext& context, const PlayerInput& input, const PlayerMoveRequest& moveRequest, float deltaTime);
    // 回転更新処理
    void UpdateRotation(const PlayerContext& context, const PlayerInput& input, const PlayerMoveRequest& moveRequest, float deltaTime);

};

#endif // PLAYER_MOVE_BEHAVIOR_H

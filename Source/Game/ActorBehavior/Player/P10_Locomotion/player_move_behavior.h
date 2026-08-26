//---------------------------------------------------
// player_move_behavior.h
// 
// Author：Miu Kitamura
// Date  ：2026/03/25
//---------------------------------------------------
#ifndef PLAYER_MOVE_BEHAVIOR_H
#define PLAYER_MOVE_BEHAVIOR_H
#include "Engine/Component/behavior_component.h"

#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_intent.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_context.h"

class PlayerMoveBehavior : public BehaviorComponent {
private:
    PlayerMoveContext m_context;
    class AnimationComponent* m_animationComponent = nullptr;

public:
    PlayerMoveBehavior() = default;
    ~PlayerMoveBehavior() = default;
    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

    /// @brief PlayerMoveBehaviorの初期化処理
    void Initialize(const class PlayerContext& playerContext, struct PlayerMoveReferences& references, class PlayerMoveSettingsAsset* settings);
    /// @brief PlayerMoveBehaviorの終了処理
    void Finalize(){
        m_context.owner = nullptr;
        m_context.transform = nullptr;
        m_context.rigidbody = nullptr;
        m_context.references.runDustParticle = nullptr;
    }

    /// @brief PlayerMoveBehaviorの更新処理を行う（PlayerBehaviorのUpdate()から呼び出す）
    void UpdateMove(
        const class PlayerContext& context, 
        const class PlayerInput& input, 
        const PlayerMoveIntent& moveIntent, 
        float deltaTime);

private:
    /// @brief 下向きのSphereCastで接地状態を判定する
    bool CheckGrounded();

    /// @brief 制御速度を適用する
    void ApplyControlVelocity(XMFLOAT3& outPosition, float deltaTime);
    /// @brief 物理速度を適用する
    void ApplyPhysicsVelocity(XMFLOAT3& outPosition, float deltaTime);

};

#endif // PLAYER_MOVE_BEHAVIOR_H

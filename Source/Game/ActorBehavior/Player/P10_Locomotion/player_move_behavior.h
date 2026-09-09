//---------------------------------------------------
// player_move_behavior.h
// 
// Author：Miu Kitamura
// Date  ：2026/03/25
//---------------------------------------------------
#ifndef PLAYER_MOVE_BEHAVIOR_H
#define PLAYER_MOVE_BEHAVIOR_H
#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_intent.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/player_move_context.h"

class PlayerMoveBehavior {
private:
    PlayerMoveContext m_context;

    bool m_requestJumpAnimation = false; // ジャンプアニメーション再生要求フラグ

    DirectX::XMFLOAT2 m_vanishOffset = { 0.0f, 0.0f }; // UIの消失点のオフセット値
    DirectX::XMFLOAT2 m_vanishOffsetVelocity = { 0.0f, 0.0f }; // UIの消失点のオフセット値の速度

public:
    PlayerMoveBehavior() = default;
    ~PlayerMoveBehavior() = default;
    void DrawInspector();

    /// @brief PlayerMoveBehaviorの初期化処理
    void Initialize(const class PlayerContext& playerContext, class PlayerMoveSettingsAsset* settings);
    /// @brief PlayerMoveBehaviorの終了処理
    void Finalize(){
        m_context.owner = nullptr;
        m_context.transform = nullptr;
        m_context.rigidbody = nullptr;
        m_context.moveEffects.Finalize();
    }

    /// @brief PlayerMoveBehaviorの更新処理を行う（PlayerBehaviorのUpdate()から呼び出す）
    void UpdateMove(
        class PlayerContext& context,
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

    // === アニメーション・UIの更新処理 ===
    void UpdateAnimation(PlayerContext& context, const PlayerMoveIntent& moveIntent, float deltaTime);
    void UpdateUi(PlayerContext& context, const PlayerMoveIntent& moveIntent, float deltaTime);

};

#endif // PLAYER_MOVE_BEHAVIOR_H

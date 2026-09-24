//---------------------------------------------------
// File  ：_/DualPistols/player_dual_pistols_slash_burst.h
// Date  ：2026/08/28
// Author：Miu Kitamura
//
// ・二丁拳銃による薙ぎ払い回転攻撃を制御するクラス
//---------------------------------------------------
#pragma once
#include <vector>
#include "Game/ActorBehavior/Base/ReactionEffects/hit_stop_sequence_task.h"
#include "Game/ActorBehavior/Player/P10_Locomotion/player_locomotion_controller.h"

struct PlayerDualPistolsContext;
class GameObject;

class PlayerDualPistolsSlashBurst {
private:
    static constexpr int MAX_ATTACK_COUNT = 3; // 最大攻撃回数

    HitStopTask m_hitStopTask;
    class AnimationComponent* m_hitStopAnimation = nullptr;
    bool m_previousCanMove = false;
    bool m_previousUseGravity = false;

    float m_attackTimer = 0.0f; // 攻撃の経過時間
    float m_fireTimer = 0.0f;   // 攻撃の発射タイマー

    bool m_isActive = false;
    bool m_isFinished = false;

    PlayerLocomotionController::LocomotionRequest m_locomotionRequest;
    int m_locomotionRequestID = -1;

    // === 攻撃のリクエストと消費の管理 ===
    bool m_wasReleaseAttackInput = false;// 攻撃入力があったかどうかのフラグ
    bool m_requestNextAttack = false;   // 次の攻撃が予約されているかどうかのフラグ
    bool m_hasBursted = false;            // 攻撃が発射されたかどうかのフラグ

    // === 攻撃の移動ステップの管理 ===
    bool m_isStepMoving = false; // 単押し攻撃の移動ステップ中かどうかのフラグ
    float m_stepMoveTimer = 0.0f; // 単押し攻撃の移動ステップの経過時間

public:
    void Initialize(PlayerDualPistolsContext& context);

    void Start(PlayerDualPistolsContext& context);
    bool StartNextStep(PlayerDualPistolsContext& context);
    void Update(PlayerDualPistolsContext& context, float deltaTime);
    void Finish(PlayerDualPistolsContext& context);

    void Cancel(PlayerDualPistolsContext& context);

    bool IsActive() const { return m_isActive; }
    bool IsFinished() const { return m_isFinished; }

    void UpdateBackground(PlayerDualPistolsContext& context);

private:
    /// @brief 攻撃中の移動ステップを更新する
    void UpdateStepMovement(PlayerDualPistolsContext& context, float deltaTime);

    /// @brief 攻撃の実行
    bool HandleSlashBurstAttack(PlayerDualPistolsContext& context, int step);
    std::vector<GameObject*> DetectAttackTarget(PlayerDualPistolsContext& context);

    // === ノックバックの計算 ===
    void CalculateKnockbackPosition(
        const PlayerDualPistolsContext& context,
        const DirectX::XMFLOAT3& targetPosition,
        const DirectX::XMFLOAT3& attackDirection,
        DirectX::XMFLOAT3& outStartPosition,
        DirectX::XMFLOAT3& outEndPosition) const;
    float CalculateDistanceBehindPlayer(
        const DirectX::XMFLOAT3& playerPosition,
        const DirectX::XMFLOAT3& playerBack,
        const DirectX::XMFLOAT3& targetPosition) const;

    bool CanRequestChainableInput(PlayerDualPistolsContext& context);

    bool IsBurstFrame(PlayerDualPistolsContext& context) const;
    bool IsInputBufferFrame(PlayerDualPistolsContext& context) const;
    bool IsChainableFrame(PlayerDualPistolsContext& context) const;
    bool IsEndMotionFrame(PlayerDualPistolsContext& context) const;

    void FireVolley(PlayerDualPistolsContext& context);
    void FireLeftPistol(PlayerDualPistolsContext& context);
    void FireRightPistol(PlayerDualPistolsContext& context);

    void BeginHitStop(PlayerDualPistolsContext& context, float duration);
    void EndHitStop();
    void HoldPosition(PlayerDualPistolsContext& context);
};

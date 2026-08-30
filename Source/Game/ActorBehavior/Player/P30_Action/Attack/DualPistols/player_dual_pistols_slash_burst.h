//---------------------------------------------------
// File  ：_/DualPistols/player_dual_pistols_slash_burst.h
// Date  ：2026/08/28
// Author：Miu Kitamura
//
// ・二丁拳銃による薙ぎ払い回転攻撃を制御するクラス
//---------------------------------------------------
#pragma once

struct PlayerDualPistolsContext;

class PlayerDualPistolsSlashBurst {
private:
    static constexpr int MAX_ATTACK_COUNT = 3; // 最大攻撃回数
    float m_attackTimer = 0.0f; // 攻撃の経過時間

    bool m_isActive = false;
    bool m_isFinished = false;

    // === 攻撃のリクエストと消費の管理 ===
    bool m_wasReleaseAttackInput = false;// 攻撃入力があったかどうかのフラグ
    bool m_requestNextAttack = false;   // 次の攻撃が予約されているかどうかのフラグ
    bool m_hasFired = false;            // 攻撃が発射されたかどうかのフラグ

    // === 攻撃の移動ステップの管理 ===
    bool m_isStepMoving = false; // 単押し攻撃の移動ステップ中かどうかのフラグ
    float m_stepMoveDuration = 0.0f; // 単押し攻撃の移動ステップの継続時間
    float m_stepMoveTimer = 0.0f; // 単押し攻撃の移動ステップの経過時間
    //float[] stepMoveDistnance = new float[4] { 7.0f, 7.0f, 7.0f, 10.0f }; // 単押し攻撃の移動ステップの距離（各攻撃段階ごとに設定）

public:
    void Initialize(PlayerDualPistolsContext& context);

    void Start(PlayerDualPistolsContext& context);
    void StartNextStep(PlayerDualPistolsContext& context);
    void Update(PlayerDualPistolsContext& context, float deltaTime);
    void Finish(PlayerDualPistolsContext& context);

    void Cancel(PlayerDualPistolsContext& context);

    bool IsActive() const { return m_isActive; }
    bool IsFinished() const { return m_isFinished; }

private:
    /// @brief 次の連鎖攻撃が入力可能かどうか
    bool CanRequestChainableInput(PlayerDualPistolsContext& context);
    /// @brief 攻撃の実行
    bool HandleSlashBurstAttack(PlayerDualPistolsContext& context, int step);

    bool IsFireFrame(PlayerDualPistolsContext& context) const;
    bool IsInputBufferFrame(PlayerDualPistolsContext& context) const;
    bool IsChainableFrame(PlayerDualPistolsContext& context) const;
    bool IsEndMotionFrame(PlayerDualPistolsContext& context) const;
};

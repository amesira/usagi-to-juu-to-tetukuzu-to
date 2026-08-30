//===================================================
// File  ：_/DualPistols/player_dual_pistols_slash_burst.cpp
// Date  ：2026/08/28
// Author：Miu Kitamura
//===================================================
#include "player_dual_pistols_slash_burst.h"
#include "player_dual_pistols_context.h"
#include "player_dual_pistols_action.h"

#include "Utility/mi_math.h"

void PlayerDualPistolsSlashBurst::Initialize(PlayerDualPistolsContext& context)
{
    // 初期化処理は必要に応じて実装する
    context.runtimeState.comboStep = 0;
}

void PlayerDualPistolsSlashBurst::Start(PlayerDualPistolsContext& context)
{
    m_isActive = true;
    m_isFinished = false;
    context.runtimeState.comboStep = 0;

    StartNextStep(context);
}

void PlayerDualPistolsSlashBurst::StartNextStep(PlayerDualPistolsContext& context)
{
    m_attackTimer = 0.0f;

    m_wasReleaseAttackInput = true;
    m_requestNextAttack = false;
    m_hasFired = false;

    // 攻撃中の移動リクエストを登録

    // 攻撃のカウントを更新
    context.runtimeState.comboStep++;
    context.runtimeState.comboStep = MiMath::Clamp(context.runtimeState.comboStep, 1, MAX_ATTACK_COUNT);

    // 移動リクエストの更新（attackStepが変動した時だけでいいはず）

    // アニメーションの再生
}

void PlayerDualPistolsSlashBurst::Update(PlayerDualPistolsContext& context, float deltaTime)
{
    m_attackTimer += deltaTime;

    // 入力解決（次の攻撃入力の予約判定）
    bool releaseAttackInput = context.runtimeState.releaseAttackInput;
    if (releaseAttackInput && !m_wasReleaseAttackInput && CanRequestChainableInput(context)) {
        m_requestNextAttack = true;
    }
    m_wasReleaseAttackInput = releaseAttackInput;

    // 攻撃時の移動ステップ更新

    // 攻撃のヒット判定処理
    if (!m_hasFired && IsFireFrame(context)) {
        HandleSlashBurstAttack(context, context.runtimeState.comboStep);
    }

    // 次の攻撃への連鎖判定
    if (m_requestNextAttack && IsChainableFrame(context)) {
        // 次の攻撃への連鎖処理
        StartNextStep(context);
        return;
    }

    // 攻撃モーションの終了判定
    if (IsEndMotionFrame(context)) {
        Finish(context);
    }
}

void PlayerDualPistolsSlashBurst::Finish(PlayerDualPistolsContext& context)
{
    m_isActive = false;
    m_isFinished = true;
}

void PlayerDualPistolsSlashBurst::Cancel(PlayerDualPistolsContext& context)
{
    m_isActive = false;
    m_isFinished = false;

    m_requestNextAttack = false;
    m_hasFired = false;
}

/// @brief 次の連鎖攻撃が入力可能かどうか
bool PlayerDualPistolsSlashBurst::CanRequestChainableInput(PlayerDualPistolsContext& context)
{
    return context.runtimeState.comboStep >= 1 && 
        context.runtimeState.comboStep < MAX_ATTACK_COUNT &&
        IsInputBufferFrame(context);
}

/// @brief 攻撃の実行
bool PlayerDualPistolsSlashBurst::HandleSlashBurstAttack(PlayerDualPistolsContext& context, int step)
{
    if (step < 1 || step > MAX_ATTACK_COUNT) return false;

    m_hasFired = true;

    // 攻撃ターゲットの検出

    // 攻撃のヒット判定処理
    {
        // ノックバック処理

        // ダメージ処理

        // ヒットストップの再生
    }

    return true;
}

#pragma region 攻撃モーションのフレーム判定
bool PlayerDualPistolsSlashBurst::IsFireFrame(PlayerDualPistolsContext& context) const
{
    return m_attackTimer >= context.settings().fireTime;
}
bool PlayerDualPistolsSlashBurst::IsInputBufferFrame(PlayerDualPistolsContext& context) const
{
    return m_attackTimer >= context.settings().inputBufferStartTime && m_attackTimer <= context.settings().endTime;
}
bool PlayerDualPistolsSlashBurst::IsChainableFrame(PlayerDualPistolsContext& context) const
{
    return m_attackTimer >= context.settings().chainTime && m_attackTimer <= context.settings().endTime;
}

bool PlayerDualPistolsSlashBurst::IsEndMotionFrame(PlayerDualPistolsContext& context) const
{
    return m_attackTimer >= context.settings().endTime;
}
#pragma endregion
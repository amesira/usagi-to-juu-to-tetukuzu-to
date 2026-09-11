//===================================================
// File  ：_/E_30_Combat/Attack/Melee/enemy_melee_attack_combat.cpp
// Date  ：2026/09/11
// Author：Miu Kitamura
//===================================================
#include "enemy_melee_attack_combat.h"
#include "Engine/Component/transform_component.h"
#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"

void EnemyMeleeAttackCombat::BeginAttack(EnemyContext& context)
{
    m_phase = EnemyMeleeAttackPhase::Jump;
    m_elapsedTime = 0.0f;
    m_jumpStartPosition = context.transform->GetPosition();
    m_landingPosition = GetAimPosition(); // 予備動作終了時に確定し、跳躍中は追尾しない。
    BeginJump(context);
}

EnemyCombatStatus EnemyMeleeAttackCombat::UpdateAttack(EnemyContext& context, float deltaTime)
{
    m_elapsedTime += deltaTime;
    switch (m_phase) {
    case EnemyMeleeAttackPhase::Jump:
        UpdateJump(context, deltaTime);
        // 仮の時間判定。実装時は着地・衝突結果と接続する。
        if (m_elapsedTime >= settings().jumpDuration) {
            m_phase = EnemyMeleeAttackPhase::Slash;
            m_elapsedTime = 0.0f;
            BeginSlash(context);
        }
        return EnemyCombatStatus::Running;
    case EnemyMeleeAttackPhase::Slash:
        UpdateSlash(context, deltaTime);
        return m_elapsedTime >= settings().slashDuration
            ? EnemyCombatStatus::Success : EnemyCombatStatus::Running;
    default:
        return EnemyCombatStatus::Failure;
    }
}

void EnemyMeleeAttackCombat::EndAttack(EnemyContext& context)
{
    ClearAttackEffects(context);
    m_phase = EnemyMeleeAttackPhase::Idle;
    m_elapsedTime = 0.0f;
}

void EnemyMeleeAttackCombat::BeginJump(EnemyContext&)
{
    // TODO: 開始位置・着地点・jumpDuration・jumpHeightを使って跳躍を開始する。
}
void EnemyMeleeAttackCombat::UpdateJump(EnemyContext&, float)
{
    // TODO: 弧を描く移動と着地/衝突を更新する。
}
void EnemyMeleeAttackCombat::BeginSlash(EnemyContext&)
{
    // TODO: スラッシュのアニメーション・攻撃判定を開始する。
}
void EnemyMeleeAttackCombat::UpdateSlash(EnemyContext&, float)
{
    // TODO: 攻撃判定の有効期間を更新する。
}
void EnemyMeleeAttackCombat::ClearAttackEffects(EnemyContext&)
{
    // TODO: 通常終了とキャンセルの両方で移動要求・攻撃判定・演出を解除する。
}

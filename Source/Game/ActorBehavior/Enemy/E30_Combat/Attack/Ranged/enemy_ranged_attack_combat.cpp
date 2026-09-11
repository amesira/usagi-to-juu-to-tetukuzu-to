#include "enemy_ranged_attack_combat.h"
#include "Game/ActorBehavior/Enemy/E00_Core/enemy_context.h"

void EnemyRangedAttackCombat::BeginAttack(EnemyContext&)
{
    m_shotsFired = 0;
    m_timeUntilNextShot = 0.0f;
}

EnemyCombatStatus EnemyRangedAttackCombat::UpdateAttack(EnemyContext& context, float deltaTime)
{
    m_timeUntilNextShot -= deltaTime;
    // 長いフレームでも指定回数を超えず、発射間隔の余りを維持する。
    while (m_shotsFired < settings().shotCount && m_timeUntilNextShot <= 0.0f) {
        SetAimPosition(context.runtimeState.combatTargetPosition);
        FireShot(context, GetAimPosition());
        ++m_shotsFired;
        m_timeUntilNextShot += settings().shotInterval;
    }
    return m_shotsFired >= settings().shotCount
        ? EnemyCombatStatus::Success : EnemyCombatStatus::Running;
}

void EnemyRangedAttackCombat::EndAttack(EnemyContext& context)
{
    ClearAttackEffects(context);
    m_timeUntilNextShot = 0.0f;
}

void EnemyRangedAttackCombat::FireShot(EnemyContext&, const DirectX::XMFLOAT3&)
{
    // TODO: 発射位置から指定された狙い位置へ向かう弾を生成する。
}

void EnemyRangedAttackCombat::ClearAttackEffects(EnemyContext&)
{
    // TODO: 射撃演出・要求を解除する。発射済みの弾は弾側が寿命を管理する。
}

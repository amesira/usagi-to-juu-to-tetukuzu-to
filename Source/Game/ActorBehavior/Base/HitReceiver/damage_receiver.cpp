//===================================================
// File  ：_/Base/HitReceiver/damage_receiver.h
// Date  ：2026/09/02
// Author：Miu Kitamura
//===================================================
#include "damage_receiver.h"

#include "Game/ActorBehavior/Base/health_behavior.h"

void DamageReceiver::Initialize(HealthBehavior* healthBehavior)
{
    m_healthBehavior = healthBehavior;
}

HitResult DamageReceiver::ReceiveDamage(const HitData& data)
{
    HitResult result = {};
    float damage = data.damage * m_damageMultiplier;

    if (m_healthBehavior) {
        m_healthBehavior->TakeDamage(damage);
        result.acceptance = HitAcceptance::Accepted;
        result.appliedDamage = damage;
        result.killed = m_healthBehavior->IsDead();
        return result;
    }

    result.acceptance = HitAcceptance::InvalidTarget;
    return result;
}

bool DamageReceiver::CanReceiveDamage(const HitData& data) const
{
    // 無敵状態やガード状態の判定を行う

    // HitAcceptanceはここで操作するべき？

    if (m_healthBehavior) {
        
    }

    return true;
}
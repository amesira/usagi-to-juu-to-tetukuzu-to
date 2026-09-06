//===================================================
// File  ：_/Base/HitReceiver/damage_receiver.h
// Date  ：2026/09/02
// Author：Miu Kitamura
//===================================================
#include "damage_receiver.h"

#include "Game/ActorBehavior/Base/health_behavior.h"

using namespace HitReceiver;

void DamageReceiver::Initialize(HealthBehavior* healthBehavior)
{
    m_healthBehavior = healthBehavior;
}

HitResult DamageReceiver::ReceiveDamage(const HitData& data)
{
    HitResult result = {};
    float damage = data.damage * m_damageMultiplier;

    // 無敵やガード判定もここで行なう
    result.acceptance = EvaluateAcceptance(data);

    if (result.WasAccepted() && m_healthBehavior) {
        m_healthBehavior->TakeDamage(damage);
        result.appliedDamage = damage;
        result.killed = m_healthBehavior->IsDead();
        return result;
    }

    return result;
}

HitAcceptance DamageReceiver::EvaluateAcceptance(const HitData& hitData) const
{
    if (!m_healthBehavior) {
        return HitAcceptance::InvalidTarget;
    }

    if (m_healthBehavior->IsDead()) {
        return HitAcceptance::Rejected;
    }

    return HitAcceptance::Accepted;
}

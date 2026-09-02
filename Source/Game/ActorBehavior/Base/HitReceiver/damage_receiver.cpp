//===================================================
// File  ：_/Base/HitReceiver/damage_receiver.h
// Date  ：2026/09/02
// Author：Miu Kitamura
//===================================================
#include "damage_receiver.h"

void DamageReceiver::Initialize(HealthBehavior* healthBehavior)
{
    m_healthBehavior = healthBehavior;
}

HitResult DamageReceiver::ReceiveDamage(const HitData&)
{
    return {};
}

bool DamageReceiver::CanReceiveDamage(const HitData&) const
{
    return false;
}

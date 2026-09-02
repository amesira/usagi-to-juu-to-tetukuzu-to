//---------------------------------------------------
// File  ：_/Base/HitReceiver/damage_receiver.h
// Date  ：2026/09/02
// Author：Miu Kitamura
// 
// ・ダメージを受け取る処理をまとめるクラス
//---------------------------------------------------
#pragma once
#include "hit_receiver_context.h"

class HealthBehavior;

class DamageReceiver {
private:
    HealthBehavior* m_healthBehavior = nullptr;
    bool m_isInvincible = false;
    float m_damageMultiplier = 1.0f;

public:
    void Initialize(HealthBehavior* healthBehavior);

    HitResult ReceiveDamage(const HitData& hitData);
    bool CanReceiveDamage(const HitData& hitData) const;

    void SetInvincible(bool isInvincible) { m_isInvincible = isInvincible; }
    bool IsInvincible() const { return m_isInvincible; }

    void SetDamageMultiplier(float multiplier) { m_damageMultiplier = multiplier; }
    float GetDamageMultiplier() const { return m_damageMultiplier; }
};

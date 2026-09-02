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
    float m_damageMultiplier = 1.0f;

public:
    void Initialize(HealthBehavior* healthBehavior);

    HitResult ReceiveDamage(const HitData& hitData);

    // === ダメージ倍率設定 ===
    void SetDamageMultiplier(float multiplier) { m_damageMultiplier = multiplier; }
    float GetDamageMultiplier() const { return m_damageMultiplier; }

private:
    HitAcceptance EvaluateAcceptance(const HitData& hitData) const;
};

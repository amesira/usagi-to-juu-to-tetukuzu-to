//---------------------------------------------------
// File  ：_/Base/HitReceiver/hit_receiver_behavior.h
// Date  ：2026/09/02
// Author：Miu Kitamura
// 
// ・攻撃が当たった時の処理をまとめるBehavior
//---------------------------------------------------
#pragma once
#include "Engine/Component/behavior_component.h"
#include "damage_receiver.h"
#include "knockback_receiver.h"
#include "hit_reaction.h"

class HealthBehavior;

class HitReceiverBehavior : public BehaviorComponent {
private:
    HealthBehavior* m_healthBehavior = nullptr;

    DamageReceiver m_damageReceiver;
    KnockbackReceiver m_knockbackReceiver;
    HitReaction m_hitReaction;

public:
    HitReceiverBehavior() = default;
    ~HitReceiverBehavior() override = default;

    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

    /// @brief 攻撃を受け取る
    HitResult ReceiveHit(const HitData& hitData);

    // === Knockback関連 ===
    bool IsKnockbackActive() const;
    void CancelKnockback();

    // === Health関連 ===
    HealthBehavior* GetHealthBehavior() const { return m_healthBehavior; }
};

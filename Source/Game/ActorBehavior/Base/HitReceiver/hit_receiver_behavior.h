//---------------------------------------------------
// File  ：_/Base/HitReceiver/hit_receiver_behavior.h
// Date  ：2026/09/02
// Author：Miu Kitamura
// 
// ・攻撃が当たった時の処理をまとめるBehavior
//---------------------------------------------------
#pragma once
#include <functional>

#include "Engine/Component/behavior_component.h"
#include "damage_receiver.h"
#include "knockback_receiver.h"

class HealthBehavior;

class HitReceiverBehavior : public BehaviorComponent {
private:
    HealthBehavior* m_healthBehavior = nullptr;

    DamageReceiver m_damageReceiver;
    KnockbackReceiver m_knockbackReceiver;

    // === 攻撃を受けた時のコールバック ===
    using OnHitCallback = std::function<void(const HitReceiver::HitData&, const HitReceiver::HitResult&)>;
    OnHitCallback m_onHitCallback = nullptr;

public:
    HitReceiverBehavior() = default;
    ~HitReceiverBehavior() override = default;

    void Start() override;
    void Update() override;
    void DrawComponentInspector() override;

    void SetOnHitCallback(OnHitCallback callback) { m_onHitCallback = callback; }

    DamageReceiver* DamageReceiver() { return &m_damageReceiver; }
    KnockbackReceiver* KnockbackReceiver() { return &m_knockbackReceiver; }

    /// @brief 攻撃を受け取る
    HitReceiver::HitResult ReceiveHit(const HitReceiver::HitData& hitData);

    // === Knockback関連 ===
    bool IsKnockbackActive() const;
    void CancelKnockback();

    // === Health関連 ===
    HealthBehavior* GetHealthBehavior() const { return m_healthBehavior; }
};

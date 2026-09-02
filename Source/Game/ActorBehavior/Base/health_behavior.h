//---------------------------------------------------
// health_behavior.h
// 
// Author：Miu Kitamura
// Date  ：2026/03/28
//---------------------------------------------------
#ifndef HEALTH_BEHAVIOR_H
#define HEALTH_BEHAVIOR_H
#include "Engine/Component/behavior_component.h"

#include <functional>

class HealthBehavior : public BehaviorComponent {
private:
    using DamageCallback = std::function<void(float currentHealth, float damage)>;

    float m_health = 100.0f;
    float m_maxHealth = 100.0f;
    DamageCallback m_onTakeDamage;

public:
    HealthBehavior() {}
    ~HealthBehavior() {}

    void Start() override;
    void Update() override;

    void DrawComponentInspector() override;

public:
    virtual void TakeDamage(float damage);
    bool IsDead() const { return m_health <= 0.0f; }

    void SetOnTakeDamageCallback(DamageCallback callback) {
        m_onTakeDamage = callback;
    }

    float GetHealthRate() const {
        if (m_maxHealth <= 0.0f) return 0.0f;
        return m_health / m_maxHealth;
    }

    // === 外部から直接行うHealthの設定 ===
    void SetMaxHealth(float maxHealth) { 
        m_maxHealth = maxHealth; 
        if (m_health > m_maxHealth) {
            m_health = m_maxHealth;
        }
    }
    float GetMaxHealth() const { return m_maxHealth; }
    void SetHealth(float health) {
        m_health = health; 
        if (m_health > m_maxHealth) {
            m_health = m_maxHealth;
        }
    }
    float GetHealth() const { return m_health; }
};

#endif // HEALTH_BEHAVIOR_H

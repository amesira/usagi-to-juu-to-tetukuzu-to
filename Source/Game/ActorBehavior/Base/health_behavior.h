//---------------------------------------------------
// health_behavior.h
// 
// Author：Miu Kitamura
// Date  ：2026/03/28
//---------------------------------------------------
#ifndef HEALTH_BEHAVIOR_H
#define HEALTH_BEHAVIOR_H
#include "Engine/Component/behavior_component.h"
#include "Game/PresBehavior/UI/ui_handle.h"

#include <functional>

class HealthBehavior : public BehaviorComponent {
private:
    using DamageCallback = std::function<void(float currentHealth, float damage)>;

    float m_health = 100.0f;
    float m_maxHealth = 100.0f;
    DamageCallback m_onTakeDamage;

    UiHandle m_healthBar;
    DirectX::XMFLOAT3 m_uiOffset = { 0.0f, 2.0f, 0.0f };
    DirectX::XMFLOAT3 m_uiScale = { 2.5f, 0.15f, 1.0f };

public:
    HealthBehavior() {}
    ~HealthBehavior() override;

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

    void SetUiOffset(const DirectX::XMFLOAT3& offset);
    void SetUiActive(bool active) {
        if (m_healthBar.IsValid()) {
            m_healthBar.SetActive(active);
        }
    }

private:
    void CreateWorldHealthUi();
    void UpdateWorldHealthUi();

};

#endif // HEALTH_BEHAVIOR_H

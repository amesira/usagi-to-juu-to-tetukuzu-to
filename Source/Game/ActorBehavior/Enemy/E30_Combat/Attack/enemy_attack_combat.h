//---------------------------------------------------
// File  ：_/E_30_Combat/Attack/enemy_attack_combat.h
// Date  ：2026/09/11
// Author：Miu Kitamura
// 
// ・予備動作・攻撃・後隙を管理し、攻撃本体を派生クラスへ委ねる
//---------------------------------------------------
#pragma once
#include "Game/ActorBehavior/Enemy/E30_Combat/enemy_combat_base.h"
#include "enemy_attack_context.h"

/// @brief 予備動作・攻撃・後隙を管理し、攻撃本体を派生クラスへ委ねる。
class EnemyAttackCombat : public EnemyCombatBase {
    EnemyAttackContext m_context;
    bool m_attackEntered = false;
    bool m_hasAttackSlot = false;

    bool m_enteredPhase = false;

public:
    EnemyAttackCombat() : EnemyCombatBase(20, false) {}

    void SetSettingsAsset(const EnemyAttackSettingsAsset* settings) { m_context.settingsAsset = settings; }

    void Initialize(EnemyContext& context) override;

    bool CanStart(const EnemyContext& context) const override;
    bool CanContinue(const EnemyContext& context) const override;
    bool IsInterruptible(const EnemyContext& context) const override;
    void UpdateBackground(EnemyContext& context, float deltaTime) override;

    void Start(EnemyContext& context) override;
    EnemyCombatStatus Update(EnemyContext& context, float deltaTime) override;
    void Finish(EnemyContext& context) override;
    void Cancel(EnemyContext& context) override;

    // === Utility ===
    bool IsInAttackRange(const EnemyContext& context) const;
    bool IsCooldownComplete() const { return m_context.runtimeState.cooldownRemaining <= 0.0f; }
    float GetMaxAttackDistance() const { return settings().maxDistance; }

protected:
    const EnemyAttackSettings::Data& settings() const { return m_context.settings(); }
    EnemyAttackContext& getContext() { return m_context; }

    // === Attack Phase ===
    const DirectX::XMFLOAT3& GetAimPosition() const { return m_context.runtimeState.aimPosition; }
    void SetAimPosition(const DirectX::XMFLOAT3& position) { m_context.runtimeState.aimPosition = position; }

    // === 派生クラスで実装する必要があるメソッド ===
    virtual void BeginWindup(EnemyContext&) {}
    virtual void UpdateWindup(EnemyContext&, float) {}
    virtual void EndWindup(EnemyContext&) {}
    virtual void BeginAttack(EnemyContext& context) = 0;
    virtual EnemyCombatStatus UpdateAttack(EnemyContext& context, float deltaTime) = 0;
    // BeginAttack後、通常終了・失敗・キャンセルのいずれでも一度だけ呼ぶ。
    virtual void EndAttack(EnemyContext& context) = 0;

private:
    void ChangePhase(EnemyAttackPhase newPhase);

    void CloseAttack(EnemyContext& context);
    void Release(EnemyContext& context);
};

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

class EnemyWaitCombat;

/// @brief 予備動作・攻撃・後隙を管理し、攻撃本体を派生クラスへ委ねる。
class EnemyAttackCombat : public EnemyCombatBase {
    EnemyAttackContext m_context;
    EnemyWaitCombat* m_waitCombat = nullptr;
    bool m_attackEntered = false;

public:
    EnemyAttackCombat() : EnemyCombatBase(20, false) {}
    // 設定とWaitは非所有参照。非実行中に設定し、この行動より長く生存させる。
    void SetSettingsAsset(const EnemyAttackSettingsAsset* settings) { m_context.settingsAsset = settings; }
    void SetWaitCombat(EnemyWaitCombat* wait) { m_waitCombat = wait; }
    const EnemyAttackContext& GetContext() const { return m_context; }
    bool CanStart(const EnemyContext& context) const override;
    bool CanContinue(const EnemyContext& context) const override;
    bool IsInterruptible(const EnemyContext& context) const override;
    void UpdateBackground(EnemyContext& context, float deltaTime) override;
    void Start(EnemyContext& context) override;
    EnemyCombatStatus Update(EnemyContext& context, float deltaTime) override;
    void Finish(EnemyContext& context) override;
    void Cancel(EnemyContext& context) override;

protected:
    const EnemyAttackSettings::Data& settings() const { return m_context.settings(); }
    const DirectX::XMFLOAT3& GetAimPosition() const { return m_context.runtimeState.aimPosition; }
    void SetAimPosition(const DirectX::XMFLOAT3& position) { m_context.runtimeState.aimPosition = position; }
    virtual void BeginWindup(EnemyContext&) {}
    virtual void EndWindup(EnemyContext&) {}
    virtual void BeginAttack(EnemyContext& context) = 0;
    virtual EnemyCombatStatus UpdateAttack(EnemyContext& context, float deltaTime) = 0;
    // BeginAttack後、通常終了・失敗・キャンセルのいずれでも一度だけ呼ぶ。
    virtual void EndAttack(EnemyContext& context) = 0;

private:
    void CloseAttack(EnemyContext& context);
    void Release(EnemyContext& context);
};

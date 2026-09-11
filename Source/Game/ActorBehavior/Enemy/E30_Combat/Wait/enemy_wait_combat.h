//---------------------------------------------------
// File  ：_/E30_Combat/Wait/enemy_wait_combat.h
// Date  ：2026/09/11
// Author：Miu Kitamura
// 
// ・射程内で停止し、クールダウン終了後にCoordinatorの攻撃許可を待つ
// ・他の敵との攻撃タイミングが重なってしまうのを防ぎたい
//---------------------------------------------------
#pragma once
#include "Game/ActorBehavior/Enemy/E30_Combat/enemy_combat_base.h"

class EnemyLocomotionController;

/// @brief 許可取得で正常終了する。通常終了では予約を保持し、中断時は取り消す。
class EnemyWaitCombat : public EnemyCombatBase {
    EnemyLocomotionController* m_controller = nullptr;
    int m_requestHandle = -1;

public:
    // 許可取得時にSuccessで終了し、Attackへ予約を引き渡す。
    EnemyWaitCombat() : EnemyCombatBase(15, false) {}

    bool CanStart(const EnemyContext& context) const override;
    bool CanContinue(const EnemyContext& context) const override;
    void Start(EnemyContext& context) override;
    EnemyCombatStatus Update(EnemyContext& context, float deltaTime) override;
    void Finish(EnemyContext& context) override;
    void Cancel(EnemyContext& context) override;

private:
    bool m_completed = false;
    bool UpdateLocomotion(const EnemyContext& context);
    void ReleaseLocomotion();

};

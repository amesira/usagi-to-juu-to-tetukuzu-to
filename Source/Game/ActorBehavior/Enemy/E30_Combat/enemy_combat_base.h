// enemy_combat_base.h
// 2026/09/10
// ・プレイヤーのPlayerActionBaseとほぼ同じ役割の、敵の戦闘行動の基底クラス
#pragma once

class EnemyContext;

enum class EnemyCombatStatus {
    Running,    // 戦闘行動が継続中
    Success,    // 戦闘行動が成功して終了
    Failure,    // 戦闘行動が失敗して終了
};

/// @brief EnemyCombatTreeが排他的に実行する戦闘行動の基底クラス。
class EnemyCombatBase {
public:
    virtual ~EnemyCombatBase() = default;

    virtual bool CanStart(const EnemyContext& context) const = 0;
    virtual void Start(EnemyContext& context) = 0;
    virtual EnemyCombatStatus Update(EnemyContext& context, float deltaTime) = 0;
    virtual void Finish(EnemyContext& context) = 0;
    virtual void Cancel(EnemyContext& context) = 0;
};

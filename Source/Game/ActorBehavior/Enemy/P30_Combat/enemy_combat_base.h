#pragma once

class EnemyContext;

enum class EnemyCombatStatus {
    Running,
    Success,
    Failure,
};

/// @brief EnemyCombatTreeが排他的に実行する戦闘行動の基底クラス。
class EnemyCombatBase {
public:
    virtual ~EnemyCombatBase() = default;

    virtual bool CanStart(const EnemyContext& context) const = 0;
    virtual void Start(EnemyContext& context) = 0;
    virtual EnemyCombatStatus Update(EnemyContext& context, float deltaTime) = 0;
    virtual void Finish(EnemyContext& context) = 0;
    virtual void Abort(EnemyContext& context) = 0;
};

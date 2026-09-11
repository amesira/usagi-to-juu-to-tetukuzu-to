#pragma once
#include "Game/ActorBehavior/Enemy/E30_Combat/enemy_combat_base.h"
#include "Game/ActorBehavior/Enemy/E30_Combat/Attack/enemy_attack_settings_asset.h"
struct EnemyAttackContext;

/// @brief スラッシュ開始を基準に一度だけBurstする。演出とは独立して判定する。
class EnemyMeleeAttackSlash {
private:
    EnemyAttackSettings::Data m_settings;

    float m_elapsedTime = 0.0f;
    bool m_hasBursted = false;
    bool m_active = false;

public:
    void Start(EnemyAttackContext& context);
    EnemyCombatStatus Update(EnemyAttackContext& context, float deltaTime);

    void Cancel();
    bool HasBursted() const { return m_hasBursted; }

private:
    void Burst(EnemyAttackContext& context);
};

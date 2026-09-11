#pragma once
#include "Game/PresBehavior/attached_effect_handle.h"

struct EnemyAttackContext;
namespace EnemyAttackSettings { struct Data; }

class EnemyMeleeAttackEffects {
private:
    AttachedEffectHandle m_slashEffect;
    int m_revisionAsset = -1;

public:
    enum class EffectsType { Slash };
    void Initialize(const EnemyAttackContext& context);
    void Update(const EnemyAttackContext& context);
    void PlayEffects(EffectsType type);
    void StopSlash();
    void Finalize();

private:
    void SynchronizeAsset(const EnemyAttackContext& context);

};

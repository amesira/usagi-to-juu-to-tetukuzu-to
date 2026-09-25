#pragma once
#include "Game/PresBehavior/attached_effect_handle.h"
#include <cstdint>
#include <limits>

struct EnemyAttackContext;
namespace EnemyAttackSettings { struct Data; }

class EnemyMeleeAttackEffects {
private:
    AttachedEffectHandle m_slashEffect;
    AttachedEffectHandle m_windupEffect;

    std::uint64_t m_revisionAsset = (std::numeric_limits<std::uint64_t>::max)();

public:
    enum class EffectsType { 
        Slash,
        BeginWindup,
        EndWindup
    };

    void Initialize(const EnemyAttackContext& context);
    void Update(const EnemyAttackContext& context);
    void PlayEffects(EffectsType type);
    void StopSlash();
    void Finalize();

private:
    void SynchronizeAsset(const EnemyAttackContext& context);

};

#include "player_dodge_effects.h"

#include "Game/ControllerBehavior/custom_post_effect_controller.h"
#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Game/ControllerBehavior/game_effect_controller.h"

namespace {
    GameEffectController* GameEffect()
    {
        return GameControllerLocator::GetGameEffectController();
    }

    CustomPostEffectController* CustomPostEffect()
    {
        return GameControllerLocator::GetCustomPostEffectController();
    }
}

void PlayerDodgeEffects::Play(PlayerDodgeEffectType type)
{
    GameEffectController* gameEffect = GameEffect();
    if (!gameEffect) return;

    switch (type) {
    case PlayerDodgeEffectType::DodgeStart:
        gameEffect->ChangeFOVTemporary(70.0f, 0.08f, 0.04f);
        break;
    case PlayerDodgeEffectType::DodgeEnd:
        gameEffect->ResetFOV(0.1f);
        break;
    }
}

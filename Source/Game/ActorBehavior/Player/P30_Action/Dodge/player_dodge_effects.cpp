#include "player_dodge_effects.h"

#include "Game/ControllerBehavior/custom_post_effect_controller.h"
#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Game/ControllerBehavior/game_feedback_controller.h"

namespace {
    GameFeedbackController* GameFeedback()
    {
        return GameControllerLocator::GetGameFeedbackController();
    }

    CustomPostEffectController* CustomPostEffect()
    {
        return GameControllerLocator::GetCustomPostEffectController();
    }
}

void PlayerDodgeEffects::Play(PlayerDodgeEffectType type)
{
    GameFeedbackController* gameEffect = GameFeedback();
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

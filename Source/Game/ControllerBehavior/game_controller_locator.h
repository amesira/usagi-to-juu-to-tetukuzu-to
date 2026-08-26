//---------------------------------------------------
// game_controller_locator.h
// 
// Author: Miu Kitamura
// Date  : 2026/04/27
//---------------------------------------------------
#ifndef GAME_CONTROLLER_LOCATOR_H
#define GAME_CONTROLLER_LOCATOR_H

class GameFeedbackController;
class CustomPostEffectController;

class GameControllerLocator {
private:
    friend class GameFeedbackController;
    friend class CustomPostEffectController;

    static inline GameFeedbackController* s_gameEffectController = nullptr;
    static inline CustomPostEffectController* s_customPostEffectController = nullptr;

public:
    static GameFeedbackController* GameFeedback() {
        return s_gameEffectController;
    }

    static CustomPostEffectController* CustomPostEffect() {
        return s_customPostEffectController;
    }
};

using Game = GameControllerLocator;

#endif // GAME_CONTROLLER_LOCATOR_H

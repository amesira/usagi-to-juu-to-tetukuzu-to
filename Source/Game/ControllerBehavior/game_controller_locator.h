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
class EnemyAIWorldController;
class WaveControllerBehavior;
class StageDecorationControllerBehavior;

class GameControllerLocator {
private:
    friend class GameFeedbackController;
    friend class CustomPostEffectController;
    friend class EnemyAIWorldController;
    friend class WaveControllerBehavior;
    friend class StageDecorationControllerBehavior;

    static inline GameFeedbackController* s_gameEffectController = nullptr;
    static inline CustomPostEffectController* s_customPostEffectController = nullptr;
    static inline EnemyAIWorldController* s_enemyAIController = nullptr;
    static inline WaveControllerBehavior* s_waveController = nullptr;
    static inline StageDecorationControllerBehavior* s_stageDecorationController = nullptr;

public:
    static StageDecorationControllerBehavior* StageDecoration() { return s_stageDecorationController; }
    static WaveControllerBehavior* Wave() { return s_waveController; }
    static EnemyAIWorldController* EnemyAIWorld() {
        return s_enemyAIController;
    }

    static GameFeedbackController* GameFeedback() {
        return s_gameEffectController;
    }

    static CustomPostEffectController* CustomPostEffect() {
        return s_customPostEffectController;
    }
};

using Game = GameControllerLocator;

#endif // GAME_CONTROLLER_LOCATOR_H

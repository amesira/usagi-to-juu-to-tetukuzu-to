#pragma once
#include <algorithm>
#include <cmath>

// エンジンに依存しないウェーブ進行。生成・破棄はControllerが担当する。
struct WaveSettings {
    int waveCount = 5;
    int firstTargetPoints = 50;
    int targetPointsIncrement = 30;
    int defeatPoints = 10;
    int maxConcurrentEnemies = 6;
    float spawnInterval = 1.5f;
    float preparationDuration = 2.0f;
    float intermissionDuration = 2.0f;
    float corpseDuration = 0.5f;
};

class WaveProgress {
public:
    enum class State { WaitingForWorld, Preparing, Battle, Clearing, Intermission, Complete, GameOver };
    State state = State::WaitingForWorld;
    int waveNumber = 0;
    int wavePoints = 0;
    int totalScore = 0;
    int targetPoints = 0;
    float remainingTime = 0.0f;

    void Update(float deltaTime, bool worldReady, int remainingEnemies, const WaveSettings& settings) {
        if (!std::isfinite(deltaTime) || deltaTime < 0.0f) return;
        if (state == State::WaitingForWorld && worldReady) {
            state = State::Preparing;
            remainingTime = (std::max)(settings.preparationDuration, 0.0f);
        }
        if (state == State::Preparing || state == State::Intermission) {
            remainingTime = (std::max)(remainingTime - deltaTime, 0.0f);
            if (worldReady && remainingTime <= 0.0f) {
                ++waveNumber;
                wavePoints = 0;
                targetPoints = (std::max)(settings.firstTargetPoints +
                    (waveNumber - 1) * settings.targetPointsIncrement, 1);
                state = State::Battle;
            }
        }
        else if (state == State::Clearing && remainingEnemies == 0) {
            if (waveNumber >= (std::max)(settings.waveCount, 1)) state = State::Complete;
            else {
                state = State::Intermission;
                remainingTime = (std::max)(settings.intermissionDuration, 0.0f);
            }
        }
    }

    void AddDefeatPoints(int points) {
        if (state != State::Battle && state != State::Clearing) return;
        points = (std::max)(points, 0);
        totalScore += points;
        wavePoints += points;
        if (state == State::Battle && wavePoints >= targetPoints) state = State::Clearing;
    }
};

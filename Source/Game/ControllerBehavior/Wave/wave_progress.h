#pragma once
#include <algorithm>
#include <cmath>
#include "Game/ControllerBehavior/Result/game_result.h"

// エンジンに依存しないウェーブ進行。生成・破棄はControllerが担当する。
struct WaveSettings {
    int waveCount = 5;
    int firstTargetPoints = 50;
    int targetPointsIncrement = 30;
    int defeatPoints = 10;
    int maxConcurrentEnemies = 6;
    float spawnInterval = 1.5f;
    float preparationDuration = 5.0f;
    float intermissionDuration = 5.0f;
    float corpseDuration = 0.5f;
    float cleanupDelay = 0.05f;
    float clearResultTransitionDelay = 2.0f;
    float gameOverResultTransitionDelay = 2.0f;
    float waveClearSlowScale = 0.2f;
    float waveClearSlowBlendDuration = 0.5f;
    float waveClearSlowHoldDuration = 1.0f;
};

class WaveProgress {
public:
    enum class State { WaitingForWorld, Preparing, Battle, ClearImpact, Intermission, Complete, GameOver };
    State state = State::WaitingForWorld;
    int waveNumber = 0;
    int wavePoints = 0;
    int totalScore = 0;
    int targetPoints = 0;
    float remainingTime = 0.0f;
    float elapsedTime = 0.0f;
    std::vector<WaveResult> results;

    void RecordCurrentWave(bool cleared) {
        if (waveNumber <= 0) return;
        WaveResult result{waveNumber, wavePoints, targetPoints, cleared};
        if (!results.empty() && results.back().waveNumber == waveNumber) results.back() = result;
        else results.push_back(result);
    }

private:
    bool m_cleanupIssued = false;
    float m_cleanupRemaining = 0;

public:
    bool ShouldCleanupEnemies() const { 
        return state == State::Intermission && !m_cleanupIssued && m_cleanupRemaining <= 0;
    }
    void MarkCleanupIssued() { 
        if (state == State::Intermission) m_cleanupIssued = true;
    }
    float GetCleanupRemaining() const { return m_cleanupRemaining; }

    void FinishClearImpact(const WaveSettings& settings) {
        if (state != State::ClearImpact) return;
        state = State::Intermission;
        m_cleanupIssued = false;
        m_cleanupRemaining = std::isfinite(settings.cleanupDelay)
            ? (std::max)(settings.cleanupDelay, 0.0f) : 0.0f;
        remainingTime = waveNumber >= (std::max)(settings.waveCount, 1)
            ? m_cleanupRemaining : (std::max)(settings.intermissionDuration, 0.0f);
    }
    void CloseClearImpactScoring() {
        if (state == State::ClearImpact) m_cleanupIssued = true;
    }

    void Update(float deltaTime, bool worldReady, int remainingEnemies, const WaveSettings& settings) {
        if (!std::isfinite(deltaTime) || deltaTime < 0.0f) return;
        if (state == State::WaitingForWorld && worldReady) {
            state = State::Preparing;
            remainingTime = (std::max)(settings.preparationDuration, 0.0f);
        }
        if (state == State::Preparing || state == State::Intermission) {
            remainingTime = (std::max)(remainingTime - deltaTime, 0.0f);
            if (state == State::Intermission) {
                m_cleanupRemaining = (std::max)(m_cleanupRemaining - deltaTime, 0.0f);
                if (!m_cleanupIssued || remainingEnemies > 0) return;
                if (waveNumber >= (std::max)(settings.waveCount, 1)) {
                    RecordCurrentWave(true);
                    state = State::Complete;
                    remainingTime = 0;
                    return;
                }
            }
            if (worldReady && remainingTime <= 0.0f) {
                if (state == State::Intermission) RecordCurrentWave(true);
                ++waveNumber;
                wavePoints = 0;
                targetPoints = (std::max)(settings.firstTargetPoints +
                    (waveNumber - 1) * settings.targetPointsIncrement, 1);
                state = State::Battle;
                m_cleanupIssued = false;
            }
        }
        if (state == State::Battle) {
            elapsedTime += deltaTime;
        }
    }

    void AddDefeatPoints(int points, const WaveSettings& settings = WaveSettings{}) {
        if (state != State::Battle
            && !((state == State::ClearImpact || state == State::Intermission) && !m_cleanupIssued)) return;
        points = (std::max)(points, 0);
        totalScore += points;
        wavePoints += points;
        if (state == State::Battle && wavePoints >= targetPoints) {
            state = State::ClearImpact;
        }
    }
};

inline float GetResultTransitionDelay(WaveProgress::State state, const WaveSettings& settings) {
    const float value = state == WaveProgress::State::Complete
        ? settings.clearResultTransitionDelay : settings.gameOverResultTransitionDelay;
    return std::isfinite(value) ? (std::max)(value, 0.0f) : 0.0f;
}

inline float GetWaveClearImpactDuration(const WaveSettings& settings) {
    const float blend = std::isfinite(settings.waveClearSlowBlendDuration)
        ? (std::max)(settings.waveClearSlowBlendDuration, 0.0f) : 0.0f;
    const float hold = std::isfinite(settings.waveClearSlowHoldDuration)
        ? (std::max)(settings.waveClearSlowHoldDuration, 0.0f) : 0.0f;
    return blend * 2.0f + hold;
}

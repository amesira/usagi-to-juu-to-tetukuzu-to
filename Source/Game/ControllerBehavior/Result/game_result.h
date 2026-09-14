#pragma once
#include <vector>
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

struct WaveResult { int waveNumber = 0, points = 0, targetPoints = 1; bool cleared = false; };
struct GameResult { std::vector<WaveResult> waves; float elapsedTime = 0; bool completed = false; };
// Scene ownership must not determine the lifetime of the last run's result.
namespace GameResultStore { inline GameResult lastRun; }

namespace ResultScoring {
struct Settings {
    int completeBonus = 500;
    float timeLimit = 300, pointsPerSecond = 5;
    int rank2 = 200, rank3 = 500, rank4 = 1000, rank5 = 1800;
};
struct Score { int base = 0, completeBonus = 0, timeBonus = 0, total = 0, rank = 1; };
inline int ClampScore(double value) {
    return static_cast<int>(std::clamp(value, 0.0, static_cast<double>((std::numeric_limits<int>::max)())));
}
inline Score Calculate(const GameResult& result, const Settings& settings) {
    Score score;
    for (const auto& wave : result.waves) score.base = ClampScore(static_cast<double>(score.base) + (std::max)(0, wave.points));
    if (result.completed) {
        score.completeBonus = (std::max)(0, settings.completeBonus);
        if (std::isfinite(result.elapsedTime) && std::isfinite(settings.timeLimit) && std::isfinite(settings.pointsPerSecond))
            score.timeBonus = ClampScore(std::floor((std::max)(0.0, static_cast<double>(settings.timeLimit) - (std::max)(0.0f, result.elapsedTime)) * (std::max)(0.0f, settings.pointsPerSecond)));
    }
    score.total = ClampScore(static_cast<double>(score.base) + score.completeBonus + score.timeBonus);
    std::array<int,4> thresholds = {settings.rank2, settings.rank3, settings.rank4, settings.rank5};
    std::sort(thresholds.begin(), thresholds.end());
    for (int threshold : thresholds) if (score.total >= (std::max)(0, threshold)) ++score.rank;
    return score;
}
}

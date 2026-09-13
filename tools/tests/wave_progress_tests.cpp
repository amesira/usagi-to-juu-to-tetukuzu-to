#include "Game/ControllerBehavior/Wave/wave_progress.h"
#include <cassert>
#include <iostream>
#include <limits>

int main() {
    WaveSettings settings;
    settings.waveCount = 2;
    settings.firstTargetPoints = 20;
    settings.targetPointsIncrement = 10;
    settings.preparationDuration = 1;
    settings.intermissionDuration = 1;
    WaveProgress progress;
    progress.Update(10, false, 0, settings);
    assert(progress.state == WaveProgress::State::WaitingForWorld);
    progress.Update(0, true, 0, settings);
    assert(progress.state == WaveProgress::State::Preparing);
    progress.Update(1, true, 0, settings);
    assert(progress.waveNumber == 1 && progress.targetPoints == 20);
    progress.AddDefeatPoints(10);
    assert(progress.state == WaveProgress::State::Battle && progress.totalScore == 10);
    progress.AddDefeatPoints(10);
    assert(progress.state == WaveProgress::State::Clearing);
    progress.Update(10, true, 1, settings);
    assert(progress.state == WaveProgress::State::Clearing);
    progress.AddDefeatPoints(10); // 残敵分も総スコアへ反映
    progress.Update(0, true, 0, settings);
    assert(progress.state == WaveProgress::State::Intermission);
    progress.Update(1, true, 0, settings);
    assert(progress.waveNumber == 2 && progress.wavePoints == 0 && progress.targetPoints == 30);
    assert(progress.totalScore == 30);
    progress.AddDefeatPoints(30);
    progress.Update(0, true, 0, settings);
    assert(progress.state == WaveProgress::State::Complete);
    progress.AddDefeatPoints(100);
    assert(progress.totalScore == 60);
    progress.state = WaveProgress::State::GameOver;
    progress.Update(10, true, 0, settings);
    progress.AddDefeatPoints(100);
    assert(progress.state == WaveProgress::State::GameOver && progress.totalScore == 60);
    progress.Update(std::numeric_limits<float>::quiet_NaN(), true, 0, settings);
    assert(progress.state == WaveProgress::State::GameOver);
    std::cout << "wave_progress_tests passed\n";
}

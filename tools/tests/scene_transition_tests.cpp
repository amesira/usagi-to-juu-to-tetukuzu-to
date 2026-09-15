#include "Engine/Core/scene_transition.h"
#include <cassert>
#include <cmath>
#include <limits>
#include <iostream>

int main() {
    SceneTransition fade;
    assert(!fade.IsActive() && fade.GetAlpha() == 0);
    assert(fade.Start(1, 2));
    assert(!fade.Start()); // Reject duplicate transition requests.
    fade.NotifyFramePresented(); // A partial fade cannot authorize scene destruction.
    fade.Update(0.5f);
    assert(fade.GetAlpha() == 0.5f && !fade.CanSwitchScene());
    fade.Update(100);
    assert(fade.GetAlpha() == 1 && !fade.CanSwitchScene());
    fade.BeginFadeIn();
    assert(fade.GetState() == SceneTransition::State::Covered);
    fade.NotifyFramePresented();
    assert(fade.CanSwitchScene());
    fade.BeginFadeIn();
    assert(fade.GetAlpha() == 1 && !fade.CanSwitchScene());
    fade.Update(1);
    assert(fade.GetAlpha() == 0.5f);
    fade.Update(1);
    assert(!fade.IsActive() && fade.GetAlpha() == 0);
    assert(fade.Start(0,0));
    fade.Update(0);
    assert(!fade.CanSwitchScene() && fade.GetAlpha() == 1);
    fade.NotifyFramePresented(); fade.BeginFadeIn(); fade.Update(0);
    assert(!fade.IsActive());
    fade.Start(); fade.Update(std::numeric_limits<float>::quiet_NaN());
    assert(fade.GetAlpha() == 0);
    fade.Reset();
    assert(!fade.CanSwitchScene() && !fade.IsActive());
    std::cout << "Scene transition tests passed\n";
}

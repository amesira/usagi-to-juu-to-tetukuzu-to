#include "Game/PresBehavior/UI/Wave/wave_ui_settings_asset.h"
#include "Utility/mi_math.h"
#include "Game/PresBehavior/UI/ui_perspective.h"
#include "Game/PresBehavior/UI/Wave/wave_ui_phase_motion.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <limits>
int main() {
    WaveUiPhaseMotion motion;
    motion.Begin(WaveProgress::State::Battle);
    motion.Update(0.5f);
    assert(motion.GetBlend(1,0.4f) == 0);
    motion.Update(0.7f);
    assert(std::abs(motion.GetBlend(1,0.4f)-0.5f) < 0.001f);
    motion.Update(1);
    assert(motion.GetBlend(1,0.4f) == 1);
    motion.Begin(WaveProgress::State::Clearing);
    assert(motion.GetBlend(1,0.4f) == 0); // Battle -> Clearing restarts at A.
    motion.Update(1);
    assert(motion.GetBlend(1,0) == 1);
    motion.Begin(WaveProgress::State::Intermission);
    motion.Update(10);
    assert(motion.GetBlend(0,0) == 0);
    motion.Reset();
    std::ifstream file("asset/Data/wave_ui_settings.data.json");
    assert(file.good());
    const auto json = nlohmann::json::parse(file);
    assert(json["type"] == "WaveUiSettingsAsset");
    WaveUiSettings::Data settings;
    assert(FieldSerialization::DeserializeFields(json["data"], settings, WaveUiSettings::GetSchema()));
    WaveUiSettings::Sanitize(settings);
    const WaveUiSettings::Data defaults;
    assert(!defaults.perspective.enabled && !defaults.chromaticEcho.enabled);
    assert(defaults.number.applyPerspective && defaults.points.text.applyChromaticEcho);
    assert(defaults.phaseScaleB.x == 0.35f && defaults.phaseMoveDelay == 1);
    auto oldData = json["data"];
    oldData.erase("perspective"); oldData.erase("chromaticEcho");
    oldData.erase("phasePlacementB"); oldData.erase("phaseScaleB");
    oldData.erase("phaseMoveDelay"); oldData.erase("phaseMoveDuration");
    oldData["number"].erase("applyPerspective"); oldData["number"].erase("applyChromaticEcho");
    WaveUiSettings::Data oldSettings;
    assert(FieldSerialization::DeserializeFields(oldData, oldSettings, WaveUiSettings::GetSchema()));
    assert(!oldSettings.perspective.enabled && oldSettings.number.applyPerspective);
    assert(oldSettings.phaseScaleB.y == 0.35f);
    settings.perspective.enabled = true;
    settings.chromaticEcho.enabled = true;
    settings.chromaticEcho.offsetDistance = 10;
    settings.phase.applyPerspective = false;
    settings.phase.applyChromaticEcho = false;
    const auto number = UiLayoutSettings::ResolveGroupPosition(settings.number.placement, {1920, 1080});
    assert(number.x == 1920 * settings.number.placement.screenAnchor.x + settings.number.placement.position.x);
    assert(number.y == 1080 * settings.number.placement.screenAnchor.y + settings.number.placement.position.y);
    const auto transform = UiPerspective::MakeTransform(settings.perspective, number, number, {1920,1080});
    assert(std::abs(transform._11 - 1) > 0.001f || std::abs(transform._22 - 1) > 0.001f);
    const auto echo = UiLayoutSettings::ResolveChromaticEcho(settings.chromaticEcho,
        settings.perspective.vanishingPoint, settings.number.placement.screenAnchor, {1920,1080});
    assert(echo.enabled && std::abs(std::hypot(echo.offset.x,echo.offset.y)-10) < 0.001f);
    const auto points = UiLayoutSettings::ResolveGroupPosition(settings.points.text.placement, {1280, 720});
    assert(points.x == 1280 * settings.points.text.placement.screenAnchor.x + settings.points.text.placement.position.x);
    assert(points.y == 720 * settings.points.text.placement.screenAnchor.y + settings.points.text.placement.position.y);
    float fill = 0, velocity = 0;
    for (int i = 0; i < 120; ++i) {
        const float previous = fill;
        fill = MiMath::SmoothDamp(fill, 1.0f, velocity, settings.points.gaugeSmoothTime, 1.0f / 60);
        assert(fill >= previous && fill <= 1);
        if (i == 0) assert(fill > 0 && fill < 1);
    }
    assert(fill > 0.999f);
    for (int i = 0; i < 120; ++i)
        fill = MiMath::SmoothDamp(fill, 0.0f, velocity, settings.points.gaugeSmoothTime, 1.0f / 60);
    assert(fill < 0.001f);
    const auto serialized = FieldSerialization::SerializeFields(settings, WaveUiSettings::GetSchema());
    WaveUiSettings::Data restored;
    assert(FieldSerialization::DeserializeFields(serialized, restored, WaveUiSettings::GetSchema()));
    assert(FieldSerialization::SerializeFields(restored, WaveUiSettings::GetSchema()) == serialized);
    settings.popupDuration = 0;
    settings.points.gaugeSmoothTime = std::numeric_limits<float>::quiet_NaN();
    settings.pulseScale = std::numeric_limits<float>::quiet_NaN();
    settings.phase.fontSize = -10;
    settings.phaseMoveDelay = -1;
    settings.phaseScaleB.x = std::numeric_limits<float>::quiet_NaN();
    WaveUiSettings::Sanitize(settings);
    assert(settings.popupDuration > 0 && std::isfinite(settings.pulseScale) && settings.phase.fontSize > 0);
    assert(settings.points.gaugeSmoothTime == 0.2f);
    assert(settings.phaseMoveDelay == 0 && settings.phaseScaleB.x == 0.35f);
    std::cout << "Wave UI settings tests passed\n";
}

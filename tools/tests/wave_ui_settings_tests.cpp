#include "Game/PresBehavior/UI/Wave/wave_ui_settings_asset.h"
#include "Utility/mi_math.h"
#include "Game/PresBehavior/UI/ui_perspective.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <limits>
int main() {
    std::ifstream file("asset/Data/wave_ui_settings.data.json");
    assert(file.good());
    const auto json = nlohmann::json::parse(file);
    assert(json["type"] == "WaveUiSettingsAsset");
    WaveUiSettings::Data settings;
    assert(FieldSerialization::DeserializeFields(json["data"], settings, WaveUiSettings::GetSchema()));
    WaveUiSettings::Sanitize(settings);
    assert(!settings.perspective.enabled && !settings.chromaticEcho.enabled);
    assert(settings.number.applyPerspective && settings.points.text.applyChromaticEcho);
    auto oldData = json["data"];
    oldData.erase("perspective"); oldData.erase("chromaticEcho");
    oldData["number"].erase("applyPerspective"); oldData["number"].erase("applyChromaticEcho");
    WaveUiSettings::Data oldSettings;
    assert(FieldSerialization::DeserializeFields(oldData, oldSettings, WaveUiSettings::GetSchema()));
    assert(!oldSettings.perspective.enabled && oldSettings.number.applyPerspective);
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
    WaveUiSettings::Sanitize(settings);
    assert(settings.popupDuration > 0 && std::isfinite(settings.pulseScale) && settings.phase.fontSize > 0);
    assert(settings.points.gaugeSmoothTime == 0.2f);
    std::cout << "Wave UI settings tests passed\n";
}

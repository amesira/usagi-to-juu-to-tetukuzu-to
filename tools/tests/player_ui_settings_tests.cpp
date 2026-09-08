#include "Game/PresBehavior/UI/Player/player_ui_settings_asset.h"
#include "Game/PresBehavior/UI/Player/player_ui_perspective.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <limits>

int main() {
    using namespace PlayerUiSettings;
    std::ifstream input("asset/Data/player_ui_settings.data.json");
    assert(input);
    const auto file = nlohmann::json::parse(input);
    Data defaults;

    Data loaded;
    assert(FieldSerialization::DeserializeFields(file.at("data"), loaded, GetSchema()));
    assert(IsValid(loaded));
    const auto hp = ResolveGroupPosition(defaults.healthBar.placement, {1920, 1080});
    const auto ammo = ResolveGroupPosition(defaults.ammoCount.placement, {1280, 720});
    assert(hp.x == 180 && hp.y == 1015);
    assert(ammo.x == 1130 && ammo.y == 580);
    loaded.healthBar.label.size = {2, 3};
    loaded.ammoCount.circleGauge.rotationDegrees = 45;
    loaded.ammoCount.weaponIcon.position = {-12, 34};
    const auto saved = FieldSerialization::SerializeFields(loaded, GetSchema());
    Data restored;
    assert(FieldSerialization::DeserializeFields(saved, restored, GetSchema()));
    assert(FieldSerialization::SerializeFields(restored, GetSchema()) == saved);
    restored.healthBar.slider.size.x = -1;
    assert(!IsValid(restored));
    restored = defaults;
    restored.ammoCount.placement.screenAnchor.y = 2;
    assert(!IsValid(restored));
    restored = defaults;
    restored.healthBar.label.rotationDegrees = std::numeric_limits<float>::infinity();
    assert(!IsValid(restored));
    restored = defaults;
    restored.perspective.cameraDistance = 0;
    assert(!IsValid(restored));
    using namespace DirectX;
    const auto NearlyEqual = [](float a, float b) { return std::abs(a - b) < 0.002f; };
    PerspectiveSettings perspective;
    const XMFLOAT2 anchor = {200, 800}, screen = {1920, 1080};
    auto matrix = PlayerUiPerspective::MakeTransform(perspective, anchor, anchor, screen);
    auto project = [&](XMFLOAT2 point) {
        XMFLOAT3 result;
        XMStoreFloat3(&result, XMVector3TransformCoord(XMVectorSet(point.x, point.y, 0, 1), XMLoadFloat4x4(&matrix)));
        return result;
    };
    auto pivot = project(anchor);
    assert(NearlyEqual(pivot.x, anchor.x) && NearlyEqual(pivot.y, anchor.y));
    XMFLOAT4 inner, outer;
    XMStoreFloat4(&inner, XMVector4Transform(XMVectorSet(300, 800, 0, 1), XMLoadFloat4x4(&matrix)));
    XMStoreFloat4(&outer, XMVector4Transform(XMVectorSet(100, 800, 0, 1), XMLoadFloat4x4(&matrix)));
    assert(inner.w > 1 && outer.w < 1); // center-facing edge recedes
    const auto unshaken = project({250, 780});
    matrix = PlayerUiPerspective::MakeTransform(perspective, anchor, {207, 795}, screen);
    const auto shaken = project({257, 775});
    assert(NearlyEqual(shaken.x, unshaken.x + 7) && NearlyEqual(shaken.y, unshaken.y - 5));
    perspective.enabled = false;
    matrix = PlayerUiPerspective::MakeTransform(perspective, anchor, anchor, screen);
    const auto flat = project({250, 780});
    assert(NearlyEqual(flat.x, 250) && NearlyEqual(flat.y, 780));
    // Old data assets can omit the new field.
    auto legacy = file.at("data");
    legacy.erase("perspective");
    Data migrated;
    assert(FieldSerialization::DeserializeFields(legacy, migrated, GetSchema()));
    assert(migrated.perspective.cameraDistance == 1200);
    const WidgetTransform bar = {{10, 20}, {200, 12}, 90};
    const auto left = MakeHealthMarker(bar, 0);
    const auto right = MakeHealthMarker(bar, 1);
    const auto fill = MakeHealthMarker(bar, 0.25f);
    assert(NearlyEqual(left.position.x, 10) && NearlyEqual(left.position.y, -80));
    assert(NearlyEqual(right.position.y, 120) && NearlyEqual(fill.position.y, -30));
    assert(fill.size.x == 2 && fill.size.y == 20 && fill.rotationDegrees == 90);
    const auto life = ResolveGroupPosition(defaults.remainingLife.placement, {1920, 1080});
    assert(life.x == 150 && life.y == 70);
    auto serialized = FieldSerialization::SerializeFields(defaults, GetSchema());
    assert(serialized["healthBar"].size() == 4); // placement plus three editable elements
    assert(serialized["ammoCount"].size() == 5); // placement plus four editable elements
    assert(serialized["remainingLife"].contains("gauge"));
    restored = defaults;
    restored.remainingLife.gauge.size.x = -1;
    assert(!IsValid(restored));
    std::cout << "Player UI settings tests passed\n";
}

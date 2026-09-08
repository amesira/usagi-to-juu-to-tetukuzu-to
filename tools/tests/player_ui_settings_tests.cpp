#include "Game/PresBehavior/UI/Player/player_ui_settings_asset.h"
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
    assert(file.at("data") == FieldSerialization::SerializeFields(defaults, GetSchema()));
    Data loaded;
    assert(FieldSerialization::DeserializeFields(file.at("data"), loaded, GetSchema()));
    assert(IsValid(loaded));
    const auto hp = ResolveGroupPosition(loaded.healthBar.placement, {1920, 1080});
    const auto ammo = ResolveGroupPosition(loaded.ammoCount.placement, {1280, 720});
    assert(hp.x == 180 && hp.y == 1015);
    assert(ammo.x == 1130 && ammo.y == 655);
    loaded.healthBar.label.size = {2, 3};
    loaded.ammoCount.fillImage.rotationDegrees = 45;
    loaded.ammoCount.backgroundImage.position = {-12, 34};
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
    std::cout << "Player UI settings tests passed\n";
}

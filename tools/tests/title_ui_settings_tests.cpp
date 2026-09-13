#include "Game/PresBehavior/UI/Title/title_ui_settings_asset.h"
#include "Game/PresBehavior/UI/Title/title_ui_presentation.h"
#include "Game/PresBehavior/UI/Player/player_ui_settings_asset.h"
#include "Game/PresBehavior/UI/Player/player_ui_perspective.h"
#include <cassert>
#include <fstream>
#include <iostream>

int main(int argc, char**) {
    TitleUiSettings::Data data;
    const auto saved = FieldSerialization::SerializeFields(data, TitleUiSettings::GetSchema());
    TitleUiSettings::Data restored;
    assert(FieldSerialization::DeserializeFields(saved, restored, TitleUiSettings::GetSchema()));
    assert(FieldSerialization::SerializeFields(restored, TitleUiSettings::GetSchema()) == saved);
    assert(!restored.popup.group.applyPerspective && restored.menu.group.applyPerspective);
    const auto version = UiLayoutSettings::ResolveGroupPosition(data.version.group.placement, {1920,1080});
    assert(version.x == 1790 && version.y == 1045);
    // Player UI JSON and schema remain compatible after extracting common settings.
    std::ifstream playerFile("asset/Data/player_ui_settings.data.json");
    auto playerJson = nlohmann::json::parse(playerFile);
    PlayerUiSettings::Data player;
    assert(FieldSerialization::DeserializeFields(playerJson.at("data"), player, PlayerUiSettings::GetSchema()));
    auto playerSaved = FieldSerialization::SerializeFields(player, PlayerUiSettings::GetSchema());
    PlayerUiSettings::Data playerRestored;
    assert(FieldSerialization::DeserializeFields(playerSaved, playerRestored, PlayerUiSettings::GetSchema()));
    assert(FieldSerialization::SerializeFields(playerRestored, PlayerUiSettings::GetSchema()) == playerSaved);
    const auto a = PlayerUiPerspective::MakeTransform(player.perspective, {100,100},{100,100},{1920,1080});
    const auto b = UiPerspective::MakeTransform(player.perspective, {100,100},{100,100},{1920,1080});
    assert(a._11 == b._11 && a._44 == b._44);
    TitleUiSelectionMotion motion;
    motion.MoveTo({0,-70},0);
    motion.MoveTo({0,70},1);
    motion.Update(0.5f);
    assert(std::abs(motion.GetPosition().y) < 0.001f);
    motion.MoveTo({130,0},0.5f);
    motion.Update(0.5f);
    assert(motion.GetPosition().x == 130 && motion.GetPosition().y == 0);
    motion.MoveTo({-130,65},0);
    assert(motion.GetPosition().x == -130);
    if (argc > 1) {
        nlohmann::json file = {{"data",saved},{"formatVersion",0},{"name","title_ui_settings.data"},{"type","TitleUiSettingsAsset"}};
        std::ofstream("asset/Data/title_ui_settings.data.json") << file.dump(4) << '\n';
    }
    std::cout << "Title UI settings and shared layout tests passed\n";
}

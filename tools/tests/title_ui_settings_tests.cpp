#include "Game/PresBehavior/UI/Title/title_ui_settings_asset.h"
#include "Game/ControllerBehavior/title_controller_settings_asset.h"
#include "Game/PresBehavior/UI/Title/title_ui_presentation.h"
#include "Game/PresBehavior/Camera/overview_camera_blend.h"
#include "Game/PresBehavior/Camera/overview_camera_settings_asset.h"
#include "Engine/Component/camera_component.h"
#include "Game/PresBehavior/UI/Player/player_ui_settings_asset.h"
#include "Game/PresBehavior/UI/Player/player_ui_perspective.h"
#include <cassert>
#include <fstream>
#include <iostream>

int main(int argc, char**) {
    OverviewCameraBlend blend;
    blend.Start({{0,40,-20},{0,0,0},60}, 1);
    auto pose = blend.Update({{0,10,-10},{0,5,0},80},0.5f);
    assert(pose.position.y == 25 && pose.fov == 70 && !blend.IsComplete());
    // Completion follows the latest moving target rather than the initial TPS pose.
    pose = blend.Update({{15,12,-5},{15,5,5},90},0.5f);
    assert(blend.IsComplete() && pose.position.x == 15 && pose.fov == 90);
    blend.Start(pose,0);
    pose = blend.Update({{20,12,-5},{20,5,5},90},0);
    assert(blend.IsComplete() && pose.position.x == 20);
    CameraComponent camera;
    assert(camera.GetRenderEnabled());
    camera.SetRenderEnabled(false);
    assert(!camera.GetRenderEnabled() && camera.GetEnable());
    std::ifstream cameraFile("asset/Data/title_camera_settings.data.json");
    const auto cameraJson = nlohmann::json::parse(cameraFile);
    OverviewCameraSettings::Data cameraSettings;
    assert(FieldSerialization::DeserializeFields(cameraJson.at("data"),cameraSettings,OverviewCameraSettings::GetSchema()));
    assert(cameraSettings.blendDuration > 0 && cameraSettings.position.y > cameraSettings.lookAt.y);
    std::ifstream titleControllerFile("asset/Data/title_controller_settings.data.json");
    const auto titleControllerJson = nlohmann::json::parse(titleControllerFile);
    TitleControllerSettings::Data titleControllerSettings;
    assert(FieldSerialization::DeserializeFields(titleControllerJson.at("data"), titleControllerSettings,
        TitleControllerSettings::GetSchema()));
    assert(titleControllerSettings.titleLogoObject.scale.x > 0);
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

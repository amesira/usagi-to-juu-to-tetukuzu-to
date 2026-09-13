#include "Game/ControllerBehavior/StageDecoration/stage_decoration_settings_asset.h"
#include "Engine/Graphics/material_resource.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <limits>

int main() {
    using namespace StageDecorationSettings;
    Data data;
    ObjectSettings object;
    object.name = "Rotating Decoration";
    object.modelPath = "asset/Model/cube.fbx";
    object.rotationEnabled = true;
    object.shader = Shader::Unlit;
    object.rotationSpeedDegrees = -30;
    data.objects.push_back(object);
    const auto json = Serialize(data);
    assert(json["objects"][0]["shader"] == "Unlit");
    Data restored;
    assert(Deserialize(json, restored));
    assert(Serialize(restored) == json);
    assert(restored.objects[0].rotationSpeedDegrees == -30);
    auto malformed = json;
    malformed["objects"][0]["shader"] = "Unsupported";
    assert(!Deserialize(malformed, restored));
    assert(Serialize(restored) == json); // 失敗時は既存値を維持
    assert(!Deserialize(nlohmann::json{{"objects", 1}}, restored));
    std::ifstream file("asset/Data/stage_decoration_settings.data.json");
    assert(file.good());
    const auto asset = nlohmann::json::parse(file);
    assert(asset["type"] == "StageDecorationSettingsAsset");
    Data supplied;
    assert(Deserialize(asset["data"], supplied));
    object.scale = {0, -1, std::numeric_limits<float>::quiet_NaN()};
    object.rotationSpeedDegrees = std::numeric_limits<float>::infinity();
    Sanitize(object);
    assert(object.scale.x > 0 && object.scale.y > 0 && object.scale.z == 1);
    assert(object.rotationSpeedDegrees == 0);
    const DirectX::XMFLOAT4 identity = {0, 0, 0, 1};
    const auto rotation = ResolveRotation(identity, {0, 2, 0}, 90);
    DirectX::XMFLOAT3 forward;
    DirectX::XMStoreFloat3(&forward, DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 0, 1, 0), DirectX::XMLoadFloat4(&rotation)));
    assert(std::abs(forward.x - 1) < 0.0001f && std::abs(forward.z) < 0.0001f);
    const auto zeroAxis = ResolveRotation(rotation, {0, 0, 0}, 90);
    assert(zeroAxis.y == rotation.y && zeroAxis.w == rotation.w);
    const auto loop = ResolveRotation(identity, {0, 1, 0}, 360);
    assert(std::abs(loop.y) < 0.0001f && std::abs(std::abs(loop.w) - 1) < 0.0001f);
    MaterialResource shared;
    MaterialInstance first, second;
    first.materialResource = second.materialResource = &shared;
    first.isOverrideShaderProgram = true;
    assert(!second.isOverrideShaderProgram && shared.shaderProgram == nullptr);
    std::cout << "Stage decoration tests passed\n";
}

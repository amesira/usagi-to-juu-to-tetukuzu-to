#include "Game/Factory/Prefab/enemy_definition_asset.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <limits>

int main() {
    const char* variants[] = {"ground_melee", "hover_ranged", "elite_ground_melee", "elite_hover_ranged"};
    for (int i = 0; i < 4; ++i) {
        const std::string path = std::string("asset/Data/enemy_") + variants[i] + ".definition.data.json";
        std::ifstream stream(path);
        assert(stream.good());
        const auto json = nlohmann::json::parse(stream);
        assert(json["type"] == "EnemyDefinitionAsset");
        EnemyDefinition::Data data;
        assert(FieldSerialization::DeserializeFields(json["data"], data, EnemyDefinition::GetSchema()));
        EnemyDefinition::Sanitize(data);
        assert(data.hover == (i % 2 == 1) && data.ranged == (i % 2 == 1));
        assert(data.maxHealth == (i >= 2 ? 200 : 100));
        assert(data.scale == (i >= 2 ? 1.2f : 1.0f));
        assert(data.material1.targetMaterialName == "Body" && data.material2.targetMaterialName == "Gear.003");
        const auto serialized = FieldSerialization::SerializeFields(data, EnemyDefinition::GetSchema());
        EnemyDefinition::Data roundtrip;
        assert(FieldSerialization::DeserializeFields(serialized, roundtrip, EnemyDefinition::GetSchema()));
        assert(FieldSerialization::SerializeFields(roundtrip, EnemyDefinition::GetSchema()) == serialized);
        for (const auto& reference : {data.agentPath, data.movePath, data.approachPath, data.attackPath}) {
            std::ifstream referenced(reference);
            assert(referenced.good());
        }
        std::ifstream moveStream(data.movePath);
        assert(nlohmann::json::parse(moveStream)["data"]["hoverEnabled"] == data.hover);
        EnemyAiAgentSettings::Data base;
        base.navigationAgent.radius = 1;
        const auto resolved = EnemyDefinition::ResolveAgent(data, base);
        assert(resolved.navigationAgent.radius == data.scale);
        assert(base.navigationAgent.radius == 1); // 共有設定を変更しない
        assert(!serialized.contains("modelPath"));
    }
    EnemyDefinition::Data invalid;
    invalid.scale = std::numeric_limits<float>::quiet_NaN();
    invalid.maxHealth = -1;
    invalid.defeatPoints = -10;
    invalid.material1.metallic = 100;
    invalid.material2.roughness = -1;
    EnemyDefinition::Sanitize(invalid);
    assert(invalid.scale == 1 && invalid.maxHealth >= 1 && invalid.defeatPoints == 0);
    assert(invalid.material1.metallic == 1 && invalid.material2.roughness == 0);
    std::cout << "Enemy definition tests passed\n";
}

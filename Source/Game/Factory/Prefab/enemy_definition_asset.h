#pragma once
#include <algorithm>
#include <cmath>
#include <string>
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Editor/Schema/field_editor.h"
#include "Game/ControllerBehavior/EnemyAI/enemy_ai_agent_settings_asset.h"

namespace EnemyDefinition {
    struct MaterialSettings {
        std::string targetMaterialName;
        DirectX::XMFLOAT4 baseColor = {1, 1, 1, 1};
        DirectX::XMFLOAT3 emissiveColor = {0, 0, 0};
        float emissiveIntensity = 0;
        float metallic = 0;
        float roughness = 0.5f;
    };
    struct Data {
        std::string displayName = "Enemy";
        bool hover = false;
        bool ranged = false;
        float maxHealth = 100;
        int defeatPoints = 10;
        float scale = 1;
        std::string agentPath = "asset/Data/enemy_ai_agent_settings.data.json";
        std::string movePath = "asset/Data/enemy_move_settings.data.json";
        std::string approachPath = "asset/Data/enemy_approach_settings.data.json";
        std::string attackPath = "asset/Data/enemy_attack_settings.data.json";
        MaterialSettings material1;
        MaterialSettings material2;
    };
    inline const auto& GetMaterialSchema() {
        static const auto schema = FieldSchema{
            MakeField("targetMaterialName", "FBX Material Name (empty: unchanged)", &MaterialSettings::targetMaterialName),
            MakeField("baseColor", "Color", &MaterialSettings::baseColor, ColorFieldOptions{}),
            MakeField("emissiveColor", "Emission Color", &MaterialSettings::emissiveColor, ColorFieldOptions{}),
            MakeField("emissiveIntensity", "Emission Intensity", &MaterialSettings::emissiveIntensity, DragFieldOptions{.dragSpeed = 0.1f, .minValue = 0, .maxValue = 100}),
            MakeField("metallic", "Metallic", &MaterialSettings::metallic, DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0, .maxValue = 1}),
            MakeField("roughness", "Roughness", &MaterialSettings::roughness, DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0, .maxValue = 1})
        };
        return schema;
    }
    inline const auto& GetSchema() {
        static const auto schema = FieldSchema{
            MakeField("displayName", "Display Name", &Data::displayName),
            MakeField("hover", "Hover", &Data::hover),
            MakeField("ranged", "Ranged", &Data::ranged),
            MakeField("maxHealth", "Max HP", &Data::maxHealth, DragFieldOptions{.dragSpeed = 1, .minValue = 1, .maxValue = 100000}),
            MakeField("defeatPoints", "Defeat Points", &Data::defeatPoints, DragFieldOptions{.dragSpeed = 1, .minValue = 0, .maxValue = 10000}),
            MakeField("scale", "Spawn Scale", &Data::scale, DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.1f, .maxValue = 10}),
            MakeField("agentPath", "AI Agent Settings", &Data::agentPath),
            MakeField("movePath", "Move Settings", &Data::movePath),
            MakeField("approachPath", "Approach Settings", &Data::approachPath),
            MakeField("attackPath", "Attack Settings", &Data::attackPath),
            MakeStructField("material1", "Material 1", &Data::material1, GetMaterialSchema(), DefaultFieldOptions{}),
            MakeStructField("material2", "Material 2", &Data::material2, GetMaterialSchema(), DefaultFieldOptions{})
        };
        return schema;
    }
    inline void Sanitize(Data& data) {
        auto finite = [](float value, float fallback) { return std::isfinite(value) ? value : fallback; };
        data.maxHealth = (std::max)(1.0f, finite(data.maxHealth, 100));
        data.defeatPoints = (std::max)(0, data.defeatPoints);
        data.scale = std::clamp(finite(data.scale, 1), 0.1f, 10.0f);
        for (auto* material : {&data.material1, &data.material2}) {
            auto& c = material->baseColor;
            c = {std::clamp(finite(c.x, 1), 0.0f, 1.0f), std::clamp(finite(c.y, 1), 0.0f, 1.0f), std::clamp(finite(c.z, 1), 0.0f, 1.0f), std::clamp(finite(c.w, 1), 0.0f, 1.0f)};
            auto& e = material->emissiveColor;
            e = {(std::max)(0.0f, finite(e.x, 0)), (std::max)(0.0f, finite(e.y, 0)), (std::max)(0.0f, finite(e.z, 0))};
            material->emissiveIntensity = (std::max)(0.0f, finite(material->emissiveIntensity, 0));
            material->metallic = std::clamp(finite(material->metallic, 0), 0.0f, 1.0f);
            material->roughness = std::clamp(finite(material->roughness, 0.5f), 0.0f, 1.0f);
        }
    }
    // Factoryと生成候補検証で共通利用。コライダーの基準半径は1。
    inline EnemyAiAgentSettings::Data ResolveAgent(const Data& definition, EnemyAiAgentSettings::Data agent) {
        agent.navigationAgent.radius = (std::max)(1.0f, agent.navigationAgent.radius) * definition.scale;
        return agent;
    }
}

class EnemyDefinitionAsset : public DataAsset {
    EnemyDefinition::Data m_data;
public:
    EnemyDefinitionAsset() : DataAsset(DataAssetTypeID::getTypeID<EnemyDefinitionAsset>(), "EnemyDefinitionAsset", 0) {}
    const EnemyDefinition::Data& GetData() const { return m_data; }
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override { return std::make_unique<EnemyDefinitionAsset>(); }
    nlohmann::json SerializeData() const override { return FieldSerialization::SerializeFields(m_data, EnemyDefinition::GetSchema()); }
    bool DeserializeDataToApply(const nlohmann::json& json) override {
        auto data = m_data;
        if (!FieldSerialization::DeserializeFields(json, data, EnemyDefinition::GetSchema())) return false;
        EnemyDefinition::Sanitize(data);
        m_data = data;
        return true;
    }
    bool DrawDataOnEditor() override {
        const bool changed = FieldEditor::DrawFields(m_data, EnemyDefinition::GetSchema());
        if (changed) EnemyDefinition::Sanitize(m_data);
        return changed;
    }
};

// enemy_ai_agent_settings_asset.h
// 2026/09/09
#pragma once
#include <DirectXMath.h>
#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Editor/Schema/field_editor.h"

namespace EnemyAiAgent {
    /// @brief ナビゲーションエージェントの設定
    struct NavigationAgentSettings {
        float radius = 0.5f;
        float maxStepHeight = 0.5f;
        float maxSlopeDegrees = 45.0f;
    };

}

namespace EnemyAiAgentSettings {
    struct Data {
        EnemyAiAgent::NavigationAgentSettings navigationAgent;
    };

    inline const auto& GetNavigationAgentSchema() {
        using Agent = EnemyAiAgent::NavigationAgentSettings;
        static const auto schema = FieldSchema{
            MakeField("radius", "Radius", &Agent::radius, DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.01f, .maxValue = 100}),
            MakeField("maxStepHeight", "Max Step Height", &Agent::maxStepHeight, DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0, .maxValue = 100}),
            MakeField("maxSlopeDegrees", "Max Slope (degrees)", &Agent::maxSlopeDegrees, DragFieldOptions{.dragSpeed = 0.1f, .minValue = 0, .maxValue = 89})
        };
        return schema;
    }

    inline const auto& GetSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("navigationAgent", "Navigation Agent", &Data::navigationAgent, GetNavigationAgentSchema(), DefaultFieldOptions{})
        };
        return schema;
    }
}

class EnemyAiAgentSettingsAsset : public DataAsset {
private:
    EnemyAiAgentSettings::Data m_data;

public:
    EnemyAiAgentSettingsAsset() : DataAsset(DataAssetTypeID::getTypeID<EnemyAiAgentSettingsAsset>(), "EnemyAiAgentSettingsAsset", 0) {}
    const EnemyAiAgentSettings::Data& GetData() const { return m_data; }
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override {
        return std::make_unique<EnemyAiAgentSettingsAsset>();
    }
    nlohmann::json SerializeData() const override {
        return FieldSerialization::SerializeFields(m_data, EnemyAiAgentSettings::GetSchema());
    }
    bool DeserializeDataToApply(const nlohmann::json& jsonData) override {
        auto loaded = m_data;
        if (!FieldSerialization::DeserializeFields(jsonData, loaded, EnemyAiAgentSettings::GetSchema())) return false;
        m_data = loaded;
        return true;
    }
    bool DrawDataOnEditor() override {
        return FieldEditor::DrawFields(m_data, EnemyAiAgentSettings::GetSchema());
    }
};
#pragma once
#include <string>
#include <DirectXMath.h>
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Editor/Schema/field_editor.h"

namespace ScoreboardEnemySettings {
struct Data {
    std::string modelPath = "asset/Model/enemy_a_model.fbx";
    std::string agentSettingsPath = "asset/Data/enemy_ai_agent_settings.data.json";
    std::string moveSettingsPath = "asset/Data/enemy_move_settings.data.json";
    DirectX::XMFLOAT3 highScoreSpawn = {-6, 2, 5};
    DirectX::XMFLOAT3 previousScoreSpawn = {6, 2, 5};
    float moveSpeed = 3.0f;
    float stopDistance = 3.0f;
    float repathInterval = 0.5f;
    float modelScale = 1.0f;
    DirectX::XMFLOAT3 boardOffset = {0, 2.5f, 0};
    DirectX::XMFLOAT3 panelScale = {4.0f, 1.4f, 1};
    DirectX::XMFLOAT4 panelColor = {0.03f, 0.03f, 0.05f, 0.85f};
    DirectX::XMFLOAT4 textColor = {1, 1, 1, 1};
    DirectX::XMFLOAT4 rankSColor = {0.35f, 0.95f, 1, 1};
    DirectX::XMFLOAT4 rankAColor = {1, 0.75f, 0.12f, 1};
    DirectX::XMFLOAT4 rankBColor = {0.75f, 0.8f, 0.85f, 1};
    DirectX::XMFLOAT4 rankCColor = {0.75f, 0.35f, 0.12f, 1};
    DirectX::XMFLOAT4 rankDColor = {0.45f, 0.45f, 0.45f, 1};
    float emissiveIntensity = 2.0f;
};
inline const auto& GetSchema() {
    using D = Data;
    static const auto schema = FieldSchema{
        MakeField("modelPath", "Model Path", &D::modelPath),
        MakeField("agentSettingsPath", "Agent Settings Path", &D::agentSettingsPath),
        MakeField("moveSettingsPath", "Move Settings Path", &D::moveSettingsPath),
        MakeField("highScoreSpawn", "High Score Spawn", &D::highScoreSpawn, DragFieldOptions{.dragSpeed=.1f, .minValue=-1000, .maxValue=1000}),
        MakeField("previousScoreSpawn", "Previous Score Spawn", &D::previousScoreSpawn, DragFieldOptions{.dragSpeed=.1f, .minValue=-1000, .maxValue=1000}),
        MakeField("moveSpeed", "Move Speed", &D::moveSpeed, DragFieldOptions{.dragSpeed=.05f, .minValue=0, .maxValue=100}),
        MakeField("stopDistance", "Stop Distance", &D::stopDistance, DragFieldOptions{.dragSpeed=.05f, .minValue=0, .maxValue=100}),
        MakeField("repathInterval", "Repath Interval", &D::repathInterval, DragFieldOptions{.dragSpeed=.01f, .minValue=.01f, .maxValue=10}),
        MakeField("modelScale", "Model Scale", &D::modelScale, DragFieldOptions{.dragSpeed=.01f, .minValue=.01f, .maxValue=100}),
        MakeField("boardOffset", "Board Offset", &D::boardOffset, DragFieldOptions{.dragSpeed=.05f, .minValue=-100, .maxValue=100}),
        MakeField("panelScale", "Panel Scale", &D::panelScale, DragFieldOptions{.dragSpeed=.05f, .minValue=.01f, .maxValue=100}),
        MakeField("panelColor", "Panel Color", &D::panelColor, ColorFieldOptions{}),
        MakeField("textColor", "Text Color", &D::textColor, ColorFieldOptions{}),
        MakeField("rankSColor", "Rank S Color", &D::rankSColor, ColorFieldOptions{}),
        MakeField("rankAColor", "Rank A Color", &D::rankAColor, ColorFieldOptions{}),
        MakeField("rankBColor", "Rank B Color", &D::rankBColor, ColorFieldOptions{}),
        MakeField("rankCColor", "Rank C Color", &D::rankCColor, ColorFieldOptions{}),
        MakeField("rankDColor", "Rank D Color", &D::rankDColor, ColorFieldOptions{}),
        MakeField("emissiveIntensity", "Emissive Intensity", &D::emissiveIntensity, DragFieldOptions{.dragSpeed=.05f, .minValue=0, .maxValue=100})
    };
    return schema;
}
}

class ScoreboardEnemySettingsAsset : public DataAsset {
    ScoreboardEnemySettings::Data m_data;
public:
    ScoreboardEnemySettingsAsset() : DataAsset(DataAssetTypeID::getTypeID<ScoreboardEnemySettingsAsset>(), "ScoreboardEnemySettingsAsset", 0) {}
    const ScoreboardEnemySettings::Data& GetData() const { return m_data; }
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override { return std::make_unique<ScoreboardEnemySettingsAsset>(); }
    nlohmann::json SerializeData() const override { return FieldSerialization::SerializeFields(m_data, ScoreboardEnemySettings::GetSchema()); }
    bool DeserializeDataToApply(const nlohmann::json& json) override { return FieldSerialization::DeserializeFields(json, m_data, ScoreboardEnemySettings::GetSchema()); }
    bool DrawDataOnEditor() override { return FieldEditor::DrawFields(m_data, ScoreboardEnemySettings::GetSchema()); }
};

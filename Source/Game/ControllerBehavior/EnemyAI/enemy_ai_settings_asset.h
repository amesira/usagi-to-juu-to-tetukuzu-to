// enemy_ai_settings_asset.h
// 2026/09/09
#pragma once
#include <DirectXMath.h>
#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Editor/Schema/field_editor.h"

namespace EnemyAi {
    // === ナビゲーショングリッド設定 ===
    struct NavigationGridSettings {
        // XZはグリッド最小端。セル中心はorigin + (座標 + 0.5) * cellSize。
        DirectX::XMFLOAT3 origin = {};
        int cellCountX = 0;
        int cellCountZ = 0;
        float cellSize = 1.0f; // 正の値を指定する
        // origin.yを基準に、この範囲を上から下へRayCastする。
        float rayTopOffset = 20.0f;
        float rayBottomOffset = -20.0f;
    };

    struct NavigationAgentSettings {
        float radius = 0.5f;
        float height = 2.0f;
        float maxStepHeight = 0.5f;
        float maxSlopeDegrees = 45.0f;
    };

}

namespace EnemyAiSettings {
    struct Data {
        EnemyAi::NavigationGridSettings navigationGrid;
        EnemyAi::NavigationAgentSettings defaultAgent;
    };

    inline const auto& GetNavigationGridSchema() {
        using Grid = EnemyAi::NavigationGridSettings;
        static const auto schema = FieldSchema{
            MakeField("origin", "Origin", &Grid::origin, DragFieldOptions{.dragSpeed=0.1f, .minValue=-10000, .maxValue=10000}),
            MakeField("cellCountX", "Cell Count X (0: Unconfigured)", &Grid::cellCountX, DragFieldOptions{.dragSpeed=1, .minValue=0, .maxValue=4096}),
            MakeField("cellCountZ", "Cell Count Z (0: Unconfigured)", &Grid::cellCountZ, DragFieldOptions{.dragSpeed=1, .minValue=0, .maxValue=4096}),
            MakeField("cellSize", "Cell Size", &Grid::cellSize, DragFieldOptions{.dragSpeed=0.1f, .minValue=0.01f, .maxValue=100}),
            MakeField("rayTopOffset", "Ray Top Offset", &Grid::rayTopOffset, DragFieldOptions{.dragSpeed=0.1f, .minValue=-10000, .maxValue=10000}),
            MakeField("rayBottomOffset", "Ray Bottom Offset", &Grid::rayBottomOffset, DragFieldOptions{.dragSpeed=0.1f, .minValue=-10000, .maxValue=10000})
        };
        return schema;
    }

    inline const auto& GetNavigationAgentSchema() {
        using Agent = EnemyAi::NavigationAgentSettings;
        static const auto schema = FieldSchema{
            MakeField("radius", "Radius", &Agent::radius, DragFieldOptions{.dragSpeed=0.01f, .minValue=0.01f, .maxValue=100}),
            MakeField("height", "Height", &Agent::height, DragFieldOptions{.dragSpeed=0.01f, .minValue=0.01f, .maxValue=100}),
            MakeField("maxStepHeight", "Max Step Height", &Agent::maxStepHeight, DragFieldOptions{.dragSpeed=0.01f, .minValue=0, .maxValue=100}),
            MakeField("maxSlopeDegrees", "Max Slope (degrees)", &Agent::maxSlopeDegrees, DragFieldOptions{.dragSpeed=0.1f, .minValue=0, .maxValue=89})
        };
        return schema;
    }

    inline const auto& GetSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("navigationGrid", "Navigation Grid", &Data::navigationGrid, GetNavigationGridSchema(), DefaultFieldOptions{}),
            MakeStructField("defaultAgent", "Default Navigation Agent", &Data::defaultAgent, GetNavigationAgentSchema(), DefaultFieldOptions{})
        };
        return schema;
    }
}

class EnemyAiSettingsAsset : public DataAsset {
private:
    EnemyAiSettings::Data m_data;

public:
    EnemyAiSettingsAsset() : DataAsset(DataAssetTypeID::getTypeID<EnemyAiSettingsAsset>(), "EnemyAiSettingsAsset", 0) {}
    const EnemyAiSettings::Data& GetData() const { return m_data; }
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override {
        return std::make_unique<EnemyAiSettingsAsset>();
    }
    nlohmann::json SerializeData() const override {
        return FieldSerialization::SerializeFields(m_data, EnemyAiSettings::GetSchema());
    }
    bool DeserializeDataToApply(const nlohmann::json& jsonData) override {
        auto loaded = m_data;
        if (!FieldSerialization::DeserializeFields(jsonData, loaded, EnemyAiSettings::GetSchema())) return false;
        m_data = loaded;
        return true;
    }
    bool DrawDataOnEditor() override {
        return FieldEditor::DrawFields(m_data, EnemyAiSettings::GetSchema());
    }
};
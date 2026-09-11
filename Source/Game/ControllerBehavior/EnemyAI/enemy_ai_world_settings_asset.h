// enemy_ai_world_settings_asset.h
// 2026/09/09
#pragma once
#include <DirectXMath.h>
#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Editor/Schema/field_editor.h"

namespace EnemyAiWorld {
    /// @brief ナビゲーショングリッドの設定
    struct NavigationGridSettings {
        DirectX::XMFLOAT2 center = { 0.0f, 0.0f }; // グリッドの中心座標（x,z）
        int cellCountX = 0;
        int cellCountZ = 0;
        float cellSize = 1.0f;

        // この範囲を上から下へRayCastする
        float rayTopPosition = 20.0f;
        float rayBottomPosition = -20.0f;

        float slopeThreshold = 15.0f; // 斜面と判定する角度（度数法）

        // グリッドの左上座標（x,z）を返す
        const DirectX::XMFLOAT2 origin() const {
            return { center.x - (cellCountX * cellSize) / 2.0f, center.y - (cellCountZ * cellSize) / 2.0f };
        }
    };
    /// @brief タクティカル情報の設定
    struct TacticalSettings {
        float updateInterval = 0.2f;
        float enemyInfluenceRadiusMultiplier = 1.5f;
        float enemyDensityCostWeight = 1.0f;
    };
}

namespace EnemyAiWorldSettings {
    struct Data {
        EnemyAiWorld::NavigationGridSettings navigationGrid;
        EnemyAiWorld::TacticalSettings tactical;
    };

    inline const auto& GetNavigationGridSchema() {
        using Grid = EnemyAiWorld::NavigationGridSettings;
        static const auto schema = FieldSchema{
            MakeField("center", "Center", &Grid::center, DragFieldOptions{.dragSpeed=0.1f, .minValue=-10000, .maxValue=10000}),
            MakeField("cellCountX", "Cell Count X (0: Unconfigured)", &Grid::cellCountX, DragFieldOptions{.dragSpeed=1, .minValue=0, .maxValue=4096}),
            MakeField("cellCountZ", "Cell Count Z (0: Unconfigured)", &Grid::cellCountZ, DragFieldOptions{.dragSpeed=1, .minValue=0, .maxValue=4096}),
            MakeField("cellSize", "Cell Size", &Grid::cellSize, DragFieldOptions{.dragSpeed=0.1f, .minValue=0.01f, .maxValue=100}),
            MakeField("rayTopPosition", "Ray Top Position", &Grid::rayTopPosition, DragFieldOptions{.dragSpeed=0.1f, .minValue=-10000, .maxValue=10000}),
            MakeField("rayBottomPosition", "Ray Bottom Position", &Grid::rayBottomPosition, DragFieldOptions{.dragSpeed=0.1f, .minValue=-10000, .maxValue=10000}),
            MakeField("slopeThreshold", "Slope Threshold (degrees)", &Grid::slopeThreshold, DragFieldOptions{.dragSpeed = 0.1f, .minValue = 0, .maxValue = 90}),
        };
        return schema;
    }

    inline const auto& GetTacticalSchema() {
        using Tactical = EnemyAiWorld::TacticalSettings;
        static const auto schema = FieldSchema{
            MakeField("updateInterval", "Update Interval", &Tactical::updateInterval, DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.01f, .maxValue = 10}),
            MakeField("enemyInfluenceRadiusMultiplier", "Enemy Influence Radius Multiplier", &Tactical::enemyInfluenceRadiusMultiplier, DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.1f, .maxValue = 10}),
            MakeField("enemyDensityCostWeight", "Enemy Density Cost Weight", &Tactical::enemyDensityCostWeight, DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0, .maxValue = 100})
        };
        return schema;
    }

    inline const auto& GetSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("navigationGrid", "Navigation Grid", &Data::navigationGrid, GetNavigationGridSchema(), DefaultFieldOptions{}),
            MakeStructField("tactical", "Tactical", &Data::tactical, GetTacticalSchema(), DefaultFieldOptions{})
        };
        return schema;
    }
}

class EnemyAiWorldSettingsAsset : public DataAsset {
private:
    EnemyAiWorldSettings::Data m_data;

public:
    EnemyAiWorldSettingsAsset() : DataAsset(DataAssetTypeID::getTypeID<EnemyAiWorldSettingsAsset>(), "EnemyAiWorldSettingsAsset", 0) {}
    const EnemyAiWorldSettings::Data& GetData() const { return m_data; }
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override {
        return std::make_unique<EnemyAiWorldSettingsAsset>();
    }
    nlohmann::json SerializeData() const override {
        return FieldSerialization::SerializeFields(m_data, EnemyAiWorldSettings::GetSchema());
    }
    bool DeserializeDataToApply(const nlohmann::json& jsonData) override {
        auto loaded = m_data;
        if (!FieldSerialization::DeserializeFields(jsonData, loaded, EnemyAiWorldSettings::GetSchema())) return false;
        m_data = loaded;
        return true;
    }
    bool DrawDataOnEditor() override {
        return FieldEditor::DrawFields(m_data, EnemyAiWorldSettings::GetSchema());
    }
};

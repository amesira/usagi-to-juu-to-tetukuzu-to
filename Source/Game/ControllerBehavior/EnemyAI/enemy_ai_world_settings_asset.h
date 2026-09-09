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

        // グリッドの左上座標（x,z）を返す
        DirectX::XMFLOAT2 origin() {
            return { center.x - (cellCountX * cellSize) / 2.0f, center.y - (cellCountZ * cellSize) / 2.0f };
        }
    };
}

namespace EnemyAiWorldSettings {
    struct Data {
        EnemyAiWorld::NavigationGridSettings navigationGrid;
    };

    inline const auto& GetNavigationGridSchema() {
        using Grid = EnemyAiWorld::NavigationGridSettings;
        static const auto schema = FieldSchema{
            MakeField("center", "Center", &Grid::center, DragFieldOptions{.dragSpeed=0.1f, .minValue=-10000, .maxValue=10000}),
            MakeField("cellCountX", "Cell Count X (0: Unconfigured)", &Grid::cellCountX, DragFieldOptions{.dragSpeed=1, .minValue=0, .maxValue=4096}),
            MakeField("cellCountZ", "Cell Count Z (0: Unconfigured)", &Grid::cellCountZ, DragFieldOptions{.dragSpeed=1, .minValue=0, .maxValue=4096}),
            MakeField("cellSize", "Cell Size", &Grid::cellSize, DragFieldOptions{.dragSpeed=0.1f, .minValue=0.01f, .maxValue=100}),
            MakeField("rayTopPosition", "Ray Top Position", &Grid::rayTopPosition, DragFieldOptions{.dragSpeed=0.1f, .minValue=-10000, .maxValue=10000}),
            MakeField("rayBottomPosition", "Ray Bottom Position", &Grid::rayBottomPosition, DragFieldOptions{.dragSpeed=0.1f, .minValue=-10000, .maxValue=10000})
        };
        return schema;
    }

    inline const auto& GetSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("navigationGrid", "Navigation Grid", &Data::navigationGrid, GetNavigationGridSchema(), DefaultFieldOptions{})
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
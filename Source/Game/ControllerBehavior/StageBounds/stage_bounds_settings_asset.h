#pragma once
#include "stage_bounds_resolver.h"
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Editor/Schema/field_editor.h"
namespace StageBounds {
inline const auto& GetSchema() {
    static const auto schema = FieldSchema{
        MakeField("min", "Minimum XYZ", &Settings::min, DragFieldOptions{.dragSpeed=.1f, .minValue=-10000, .maxValue=10000}),
        MakeField("max", "Maximum XYZ", &Settings::max, DragFieldOptions{.dragSpeed=.1f, .minValue=-10000, .maxValue=10000}),
        MakeField("enableMinX", "Enable Min X", &Settings::enableMinX),
        MakeField("enableMaxX", "Enable Max X", &Settings::enableMaxX),
        MakeField("enableMinY", "Enable Floor (Min Y)", &Settings::enableMinY),
        MakeField("enableMaxY", "Enable Ceiling (Max Y)", &Settings::enableMaxY),
        MakeField("enableMinZ", "Enable Min Z", &Settings::enableMinZ),
        MakeField("enableMaxZ", "Enable Max Z", &Settings::enableMaxZ)
    }; return schema;
}
}
class StageBoundsSettingsAsset : public DataAsset {
    StageBounds::Settings m_data;
public:
    StageBoundsSettingsAsset() : DataAsset(DataAssetTypeID::getTypeID<StageBoundsSettingsAsset>(), "StageBoundsSettingsAsset", 0) {}
    const StageBounds::Settings& GetData() const { return m_data; }
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override { return std::make_unique<StageBoundsSettingsAsset>(); }
    nlohmann::json SerializeData() const override { return FieldSerialization::SerializeFields(m_data, StageBounds::GetSchema()); }
    bool DeserializeDataToApply(const nlohmann::json& json) override {
        auto loaded = m_data;
        if (!FieldSerialization::DeserializeFields(json, loaded, StageBounds::GetSchema())) return false;
        StageBounds::Sanitize(loaded); m_data = loaded; return true;
    }
    bool DrawDataOnEditor() override {
        bool changed = FieldEditor::DrawFields(m_data, StageBounds::GetSchema());
        if (changed) StageBounds::Sanitize(m_data);
        return changed;
    }
};

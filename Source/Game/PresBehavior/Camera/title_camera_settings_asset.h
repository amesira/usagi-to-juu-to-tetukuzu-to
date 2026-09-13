#pragma once
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Editor/Schema/field_editor.h"
#include <DirectXMath.h>

namespace TitleCameraSettings {
    struct Data {
        DirectX::XMFLOAT3 position = {0, 45, -25};
        DirectX::XMFLOAT3 lookAt = {0, 0, 10};
        float fovDegrees = 60;
        float blendDuration = 1.2f;
    };
    inline const auto& GetSchema() {
        static const auto schema = FieldSchema{
            MakeField("position", "Overhead Position", &Data::position, DragFieldOptions{.dragSpeed=0.1f, .minValue=-10000, .maxValue=10000}),
            MakeField("lookAt", "Overhead Look At", &Data::lookAt, DragFieldOptions{.dragSpeed=0.1f, .minValue=-10000, .maxValue=10000}),
            MakeField("fovDegrees", "FOV (degrees)", &Data::fovDegrees, DragFieldOptions{.dragSpeed=0.1f, .minValue=1, .maxValue=179}),
            MakeField("blendDuration", "Blend Duration (s)", &Data::blendDuration, DragFieldOptions{.dragSpeed=0.01f, .minValue=0, .maxValue=10})
        }; return schema;
    }
}
class TitleCameraSettingsAsset : public DataAsset {
    TitleCameraSettings::Data m_data;
public:
    TitleCameraSettingsAsset() : DataAsset(DataAssetTypeID::getTypeID<TitleCameraSettingsAsset>(), "TitleCameraSettingsAsset", 0) {}
    const TitleCameraSettings::Data& GetData() const { return m_data; }
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override { return std::make_unique<TitleCameraSettingsAsset>(); }
    nlohmann::json SerializeData() const override { return FieldSerialization::SerializeFields(m_data, TitleCameraSettings::GetSchema()); }
    bool DeserializeDataToApply(const nlohmann::json& json) override {
        auto loaded = m_data;
        if (!FieldSerialization::DeserializeFields(json, loaded, TitleCameraSettings::GetSchema())) return false;
        m_data = loaded; return true;
    }
    bool DrawDataOnEditor() override { return FieldEditor::DrawFields(m_data, TitleCameraSettings::GetSchema()); }
};

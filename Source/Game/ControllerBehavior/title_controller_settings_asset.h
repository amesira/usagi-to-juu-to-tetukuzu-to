#pragma once

#include <DirectXMath.h>
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Editor/Schema/field_editor.h"
#include "Engine/Asset/Schema/field_struct.h"

namespace TitleControllerSettings {
    struct TitleLogoObject {
        DirectX::XMFLOAT3 position = {-9.9f, 12.5f, 17.1f};
        DirectX::XMFLOAT3 rotation = {115.0f, -48.0f, 0.0f};
        DirectX::XMFLOAT3 scale = {5.0f, 5.0f, 5.0f};
    };

    struct Data {
        TitleLogoObject titleLogoObject;
    };

    inline const auto& GetTitleLogoObjectSchema() {
        static const auto schema = FieldSchema{
            MakeField("position", "Position", &TitleLogoObject::position,
                DragFieldOptions{.dragSpeed=0.1f, .minValue=-1000.0f, .maxValue=1000.0f}),
            MakeField("rotation", "Rotation", &TitleLogoObject::rotation,
                DragFieldOptions{.dragSpeed=0.1f, .minValue=-360.0f, .maxValue=360.0f}),
            MakeField("scale", "Scale", &TitleLogoObject::scale,
                DragFieldOptions{.dragSpeed=0.01f, .minValue=0.01f, .maxValue=100.0f})
        };
        return schema;
    }

    inline const auto& GetSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("titleLogoObject", "Title Logo Object",
                &Data::titleLogoObject, GetTitleLogoObjectSchema(), DefaultFieldOptions{})
        };
        return schema;
    }
}

class TitleControllerSettingsAsset : public DataAsset {
    TitleControllerSettings::Data m_data;
public:
    TitleControllerSettingsAsset()
        : DataAsset(DataAssetTypeID::getTypeID<TitleControllerSettingsAsset>(), "TitleControllerSettingsAsset", 0) {}
    const TitleControllerSettings::Data& GetData() const { return m_data; }
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override {
        return std::make_unique<TitleControllerSettingsAsset>();
    }
    nlohmann::json SerializeData() const override {
        return FieldSerialization::SerializeFields(m_data, TitleControllerSettings::GetSchema());
    }
    bool DeserializeDataToApply(const nlohmann::json& json) override {
        auto loaded = m_data;
        if (!FieldSerialization::DeserializeFields(json, loaded, TitleControllerSettings::GetSchema())) return false;
        m_data = loaded;
        return true;
    }
    bool DrawDataOnEditor() override {
        return FieldEditor::DrawFields(m_data, TitleControllerSettings::GetSchema());
    }
};

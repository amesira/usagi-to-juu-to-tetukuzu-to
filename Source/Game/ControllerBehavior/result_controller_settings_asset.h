#pragma once
#include <string>
#include <DirectXMath.h>

#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Editor/Schema/field_editor.h"
#include "Engine/Asset/Schema/field_struct.h"

namespace ResultControllerSettings {
    struct ResultTextObject {
        DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
        DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };
        DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
    };

    struct Data {
        ResultTextObject resultTextObject;
    };

    inline const auto& GetResultTextObjectSchema() {
        static const auto schema = FieldSchema{
            MakeField("position", "Position", &ResultTextObject::position,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = -1000.0f, .maxValue = 1000.0f }),
            MakeField("rotation", "Rotation", &ResultTextObject::rotation,
                DragFieldOptions{.dragSpeed = 0.1f, .minValue = -360.0f, .maxValue = 360.0f }),
            MakeField("scale", "Scale", &ResultTextObject::scale,
                DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.01f, .maxValue = 100.0f })
        };
        return schema;
    }

    inline const auto& GetSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("resultTextObject", "Result Text Object", 
                &Data::resultTextObject, GetResultTextObjectSchema(), DefaultFieldOptions{})
        };
        return schema;
    }
}

class ResultControllerSettingsAsset : public DataAsset {
    ResultControllerSettings::Data m_data;
public:
    ResultControllerSettingsAsset()
        : DataAsset(DataAssetTypeID::getTypeID<ResultControllerSettingsAsset>(), "ResultControllerSettingsAsset", 0) {}

    const ResultControllerSettings::Data& GetData() const { return m_data; }
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override {
        return std::make_unique<ResultControllerSettingsAsset>();
    }
    nlohmann::json SerializeData() const override {
        return FieldSerialization::SerializeFields(m_data, ResultControllerSettings::GetSchema());
    }
    bool DeserializeDataToApply(const nlohmann::json& json) override {
        auto loaded = m_data;
        if (!FieldSerialization::DeserializeFields(json, loaded, ResultControllerSettings::GetSchema())) return false;
        m_data = loaded;
        return true;
    }
    bool DrawDataOnEditor() override {
        return FieldEditor::DrawFields(m_data, ResultControllerSettings::GetSchema());
    }
};

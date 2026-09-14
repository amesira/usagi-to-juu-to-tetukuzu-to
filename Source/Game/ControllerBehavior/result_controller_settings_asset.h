#pragma once

#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Editor/Schema/field_editor.h"

namespace ResultControllerSettings {
    struct Data {
        // リザルト進行用の設定項目をここに追加する。
    };

    inline const auto& GetSchema() {
        // Dataに追加した項目をMakeFieldで登録する。
        static const FieldSchema<> schema{};
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

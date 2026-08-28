//---------------------------------------------------
// File  ：_/DualPistols/player_dual_pistols_settings_asset.h
// Date  ：2026/08/28
// Author：Miu Kitamura
//
// ・二丁拳銃アクションの設定Asset
//---------------------------------------------------
#pragma once

#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Editor/Schema/field_editor.h"

namespace PlayerDualPistolsSettings {
    struct Data {
    };

    inline static const auto& GetSchema()
    {
        static const auto& schema = FieldSchema{
            MakeHeaderField("Dual Pistols Settings"),
        };
        return schema;
    }
}

class PlayerDualPistolsSettingsAsset : public DataAsset {
private:
    PlayerDualPistolsSettings::Data m_data;

    static constexpr std::string_view s_assetTypeName = "PlayerDualPistolsSettingsAsset";
    static constexpr int s_supportedFormatVersion = 0;

public:
    PlayerDualPistolsSettingsAsset()
        : DataAsset(
            DataAssetTypeID::getTypeID<PlayerDualPistolsSettingsAsset>(),
            s_assetTypeName,
            s_supportedFormatVersion)
    {
    }

    ~PlayerDualPistolsSettingsAsset() override = default;

    const PlayerDualPistolsSettings::Data& GetData() const { return m_data; }

    std::unique_ptr<DataAsset> CreateDefaultInstance() const override
    {
        auto clone = std::make_unique<PlayerDualPistolsSettingsAsset>();
        clone->SetHeader(AssetHeader{ s_assetTypeName.data(), s_supportedFormatVersion, "" });
        clone->m_data = PlayerDualPistolsSettings::Data{};
        return clone;
    }

    nlohmann::json SerializeData() const override
    {
        return FieldSerialization::SerializeFields(
            m_data,
            PlayerDualPistolsSettings::GetSchema());
    }

    bool DeserializeDataToApply(const nlohmann::json& jsonData) override
    {
        PlayerDualPistolsSettings::Data loaded = m_data;
        if (!FieldSerialization::DeserializeFields(
            jsonData,
            loaded,
            PlayerDualPistolsSettings::GetSchema())) {
            return false;
        }

        m_data = loaded;
        return true;
    }

    bool DrawDataOnEditor() override
    {
        return FieldEditor::DrawFields(
            m_data,
            PlayerDualPistolsSettings::GetSchema());
    }
};

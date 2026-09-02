// player_prefab_settings_asset.h
#pragma once
#include "Engine/Asset/Schema/field_master.h"

#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"

#include "Engine/Editor/Schema/field_editor.h"

namespace PlayerPrefabSettings {
    using json = nlohmann::json;

    /// @brief プレイヤーのプレハブ設定を表す構造体
    struct Data {
        XMFLOAT3 scaling = { 1.0f, 1.0f, 1.0f };

        XMFLOAT3 colliderScale = { 1.0f, 1.0f, 1.0f };
        XMFLOAT3 colliderCenter = { 0.0f, 0.0f, 0.0f };
    };

    /// @brief AttachedEffectSettingsのFieldSchema
    inline static const auto& GetSchema() 
    {
        static const auto& schema = FieldSchema{
            MakeField(
                "scaling",
                "Scaling",
                &Data::scaling,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.1f,
                    .maxValue = 10.0f }),
            MakeField(
                "colliderScale",
                "Collider Scale",
                &Data::colliderScale,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.1f,
                    .maxValue = 10.0f }),
            MakeField(
                "colliderCenter",
                "Collider Center",
                &Data::colliderCenter,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = -10.0f,
                    .maxValue = 10.0f })
        };

        return schema;
    };

}

class PlayerPrefabSettingsAsset : public DataAsset {
private:
    PlayerPrefabSettings::Data m_data;

    static constexpr std::string_view s_assetTypeName = "PlayerPrefabSettingsAsset";
    static constexpr int s_supportedFormatVersion = 0;

public:
    PlayerPrefabSettingsAsset() : DataAsset(
        DataAssetTypeID::getTypeID<PlayerPrefabSettingsAsset>(),
        s_assetTypeName, 
        s_supportedFormatVersion) {}
    virtual ~PlayerPrefabSettingsAsset() = default;

    // === DataAssetのオーバーライド ===
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override {
        return std::make_unique<PlayerPrefabSettingsAsset>();
    }

    // === シリアライズ、デシリアライズ ===
    nlohmann::json SerializeData() const override
    {
        nlohmann::json jsonData = FieldSerialization::SerializeFields(m_data, PlayerPrefabSettings::GetSchema());
        return jsonData;
    }

    bool DeserializeDataToApply(const nlohmann::json& jsonData) override
    {
        PlayerPrefabSettings::Data loaded = m_data;
        if (!FieldSerialization::DeserializeFields(jsonData, loaded, PlayerPrefabSettings::GetSchema())) {
            return false;
        }

        // デシリアライズに成功した場合は、実データに反映する
        m_data = loaded;
        return true;
    }

    // == Editor用の描画 ===
    bool DrawDataOnEditor() override
    {
        bool changed = false;
        changed |= FieldEditor::DrawFields(m_data, PlayerPrefabSettings::GetSchema());
        return changed;
    }

    /// @brief PlayerPrefabSettingsのデータを取得する
    const PlayerPrefabSettings::Data& GetData() const { return m_data; }

};

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
        float startRapidFireDelay = 0.1f;

        // === Aim / Rapid Fire ===
        float aimMaxDistance = 100.0f;
        float rapidFireInterval = 0.1f;
        float bulletSpeed = 50.0f;
        float bulletRadius = 0.1f;
        float bulletLifetime = 3.0f;
        float bulletSpawnForwardOffset = 0.2f;

        // === Rapid Fire Animation ===
        float rapidFireAimBlendDownFullDirectionY = -0.5f;
        float rapidFireAimBlendDownStartDirectionY = -0.2f;
        float rapidFireAimBlendUpStartDirectionY = 0.1f;
        float rapidFireAimBlendUpFullDirectionY = 0.5f;
        float rapidFireMoveBlendSmoothTime = 0.1f;
        float rapidFireAimBlendSmoothTime = 0.1f;

        // === Slash Burst ===
        float fireTime = 0.1f;
        float inputBufferStartTime = 0.1f;
        float chainTime = 0.1f;
        float endTime = 0.5f;
    };

    inline static const auto& GetSchema()
    {
        static const auto& schema = FieldSchema{
            MakeHeaderField("Dual Pistols Settings"),
            MakeField("startRapidFireDelay", "Start Rapid Fire Delay", &Data::startRapidFireDelay,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 2.0f }),

            MakeHeaderField("Aim / Rapid Fire"),
            MakeField("aimMaxDistance", "Aim Max Distance", &Data::aimMaxDistance,
                DragFieldOptions{ .dragSpeed = 1.0f, .minValue = 0.0f, .maxValue = 1000.0f }),
            MakeField("rapidFireInterval", "Rapid Fire Interval", &Data::rapidFireInterval,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.01f, .maxValue = 2.0f }),
            MakeField("bulletSpeed", "Bullet Speed", &Data::bulletSpeed,
                DragFieldOptions{ .dragSpeed = 1.0f, .minValue = 0.0f, .maxValue = 1000.0f }),
            MakeField("bulletRadius", "Bullet Radius", &Data::bulletRadius,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f }),
            MakeField("bulletLifetime", "Bullet Lifetime", &Data::bulletLifetime,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 60.0f }),
            MakeField("bulletSpawnForwardOffset", "Bullet Spawn Forward Offset", &Data::bulletSpawnForwardOffset,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f }),

            MakeHeaderField("Rapid Fire Animation"),
            MakeField("rapidFireAimBlendDownFullDirectionY", "Aim Blend Down Full Direction Y",
                &Data::rapidFireAimBlendDownFullDirectionY,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = -1.0f, .maxValue = 1.0f }),
            MakeField("rapidFireAimBlendDownStartDirectionY", "Aim Blend Down Start Direction Y",
                &Data::rapidFireAimBlendDownStartDirectionY,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = -1.0f, .maxValue = 1.0f }),
            MakeField("rapidFireAimBlendUpStartDirectionY", "Aim Blend Up Start Direction Y",
                &Data::rapidFireAimBlendUpStartDirectionY,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = -1.0f, .maxValue = 1.0f }),
            MakeField("rapidFireAimBlendUpFullDirectionY", "Aim Blend Up Full Direction Y",
                &Data::rapidFireAimBlendUpFullDirectionY,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = -1.0f, .maxValue = 1.0f }),
            MakeField("rapidFireMoveBlendSmoothTime", "Move Blend Smooth Time",
                &Data::rapidFireMoveBlendSmoothTime,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 1.0f }),
            MakeField("rapidFireAimBlendSmoothTime", "Aim Blend Smooth Time",
                &Data::rapidFireAimBlendSmoothTime,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 1.0f }),

            MakeHeaderField("Slash Burst"),
            MakeField("fireTime", "Fire Time", &Data::fireTime,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f }),
            MakeField("inputBufferStartTime", "Input Buffer Start Time", &Data::inputBufferStartTime,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f }),
            MakeField("chainTime", "Chain Time", &Data::chainTime,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f }),
            MakeField("endTime", "End Time", &Data::endTime,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f }),
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

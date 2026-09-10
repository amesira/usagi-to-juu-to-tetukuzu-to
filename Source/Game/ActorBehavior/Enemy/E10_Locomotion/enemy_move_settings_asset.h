// enemy_move_settings_asset.h
// 2026/09/10
#pragma once

#include <string>
#include <DirectXMath.h>

#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Editor/Schema/field_editor.h"

namespace EnemyMoveSettings {
    struct AttachedEffectSettings {
        std::string particleAssetPath = "asset/Particle/run_dust.particle.json";
        DirectX::XMFLOAT3 positionOffset = { 0.0f, 0.0f, 0.0f };
    };

    struct Data {
        float moveSpeed = 6.0f;
        float acceleration = 20.0f;
        float deceleration = 30.0f;
        float rotationSpeed = 10.0f;
        float waypointReachDistance = 0.2f;
        float runEffectMinSpeed = 0.1f;
        AttachedEffectSettings runEffect;
    };

    inline const auto& GetAttachedEffectSettingsSchema()
    {
        using Settings = AttachedEffectSettings;
        static const auto schema = FieldSchema{
            MakeField("particleAssetPath", "Particle Asset Path", &Settings::particleAssetPath),
            MakeField("positionOffset", "Position Offset", &Settings::positionOffset,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = -10.0f, .maxValue = 10.0f }),
        };
        return schema;
    }

    inline const auto& GetSchema()
    {
        using Settings = Data;
        static const auto schema = FieldSchema{
            MakeHeaderField("Movement"),
            MakeField("moveSpeed", "Move Speed", &Settings::moveSpeed,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("acceleration", "Acceleration", &Settings::acceleration,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("deceleration", "Deceleration", &Settings::deceleration,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("rotationSpeed", "Rotation Speed", &Settings::rotationSpeed,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeField("waypointReachDistance", "Waypoint Reach Distance", &Settings::waypointReachDistance,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.01f, .maxValue = 10.0f }),
            MakeHeaderField("Effects"),
            MakeField("runEffectMinSpeed", "Run Effect Min Speed", &Settings::runEffectMinSpeed,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f }),
            MakeStructField("runEffect", "Run Effect", &Settings::runEffect,
                GetAttachedEffectSettingsSchema(), DefaultFieldOptions{}),
        };
        return schema;
    }
}

class EnemyMoveSettingsAsset : public DataAsset {
    EnemyMoveSettings::Data m_data;

public:
    EnemyMoveSettingsAsset()
        : DataAsset(DataAssetTypeID::getTypeID<EnemyMoveSettingsAsset>(), "EnemyMoveSettingsAsset", 0) {}

    const EnemyMoveSettings::Data& GetData() const { return m_data; }

    std::unique_ptr<DataAsset> CreateDefaultInstance() const override
    {
        return std::make_unique<EnemyMoveSettingsAsset>();
    }

    nlohmann::json SerializeData() const override
    {
        return FieldSerialization::SerializeFields(m_data, EnemyMoveSettings::GetSchema());
    }

    bool DeserializeDataToApply(const nlohmann::json& jsonData) override
    {
        auto loaded = m_data;
        if (!FieldSerialization::DeserializeFields(jsonData, loaded, EnemyMoveSettings::GetSchema())) return false;
        m_data = loaded;
        return true;
    }

    bool DrawDataOnEditor() override
    {
        return FieldEditor::DrawFields(m_data, EnemyMoveSettings::GetSchema());
    }
};

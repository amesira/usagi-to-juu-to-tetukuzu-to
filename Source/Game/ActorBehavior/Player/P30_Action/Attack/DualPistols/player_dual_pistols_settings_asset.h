//---------------------------------------------------
// File  ：_/DualPistols/player_dual_pistols_settings_asset.h
// Date  ：2026/08/28
// Author：Miu Kitamura
//
// ・二丁拳銃アクションの設定Asset
//---------------------------------------------------
#pragma once
#include <DirectXMath.h>
#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Editor/Schema/field_editor.h"
#include "Utility/mi_curve.h"

namespace PlayerDualPistolsSettings {
    struct SlashBurstEffectTransformSettings {
        DirectX::XMFLOAT3 position{};
        DirectX::XMFLOAT3 rotation{};
    };

    inline static const auto& GetSlashBurstEffectTransformSettingsSchema()
    {
        static const auto& schema = FieldSchema{
            MakeField(
                "position",
                "Position",
                &SlashBurstEffectTransformSettings::position,
                DragFieldOptions{
                    .dragSpeed = 0.01f,
                    .minValue = -20.0f,
                    .maxValue = 20.0f }),
            MakeField(
                "rotation",
                "Rotation",
                &SlashBurstEffectTransformSettings::rotation,
                DragFieldOptions{
                    .dragSpeed = 0.01f,
                    .minValue = -360.0f,
                    .maxValue = 360.0f }),
        };
        return schema;
    }

    struct Data {
        float startRapidFireDelay = 0.1f;

        // === Aim / Rapid Fire ===
        float aimMaxDistance = 100.0f;
        float rapidFireInterval = 0.1f;
        float rapidFireMoveSpeedMultiplier = 1.0f;
        float rapidFireJumpPowerMultiplier = 1.0f;
        
        // === Rapid Fire Animation ===
        float rapidFireAimBlendDownFullDirectionY = -0.5f;
        float rapidFireAimBlendDownStartDirectionY = -0.2f;
        float rapidFireAimBlendUpStartDirectionY = 0.1f;
        float rapidFireAimBlendUpFullDirectionY = 0.5f;

        // === Slash Burst ===
        float slashBurstFireInterval = 0.1f;
        float slashBurstFireDuration = 0.2f;
        float burstTime = 0.1f;
        float inputBufferStartTime = 0.1f;
        float chainTime = 0.1f;
        float endTime = 0.5f;

        // === Slash Burst Effects ===
        std::string slashBurstEffectAssetPath =
            "asset/MeshEffect/player_slash_burst_1_effect.mesh_effect.json";
        SlashBurstEffectTransformSettings slashBurst1Effect;
        SlashBurstEffectTransformSettings slashBurst2Effect;
        SlashBurstEffectTransformSettings slashBurst3LeftEffect;
        SlashBurstEffectTransformSettings slashBurst3RightEffect;
        float slashBurstEffectScale = 1.0f;

        // === Slash Burst Movement ===
        float stepMoveDuration = 0.2f;
        float stepMoveDistance = 1.0f;
        float finalStepMoveDuration = 0.3f;
        float finalStepMoveDistance = 1.5f;
        float slashBurstKnockbackDistance = 5.0f;
        float slashBurstMaxKnockbackDistanceFromPlayer = 8.0f;
        float slashBurstKnockbackStartMargin = 0.5f;
        float slashBurstKnockbackDuration = 0.5f;

        // === Bullet Spread ===
        float bulletSpeed = 50.0f;
        float bulletRadius = 0.1f;
        float bulletLifetime = 3.0f;
        float bulletSpawnForwardOffset = 0.2f;
    };

    inline static const auto& GetSchema()
    {
        static const auto& schema = FieldSchema{
            MakeHeaderField("Dual Pistols Settings"),
            MakeField("startRapidFireDelay", "Start Rapid Fire Delay", &Data::startRapidFireDelay,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 2.0f }),

            // === Aim / Rapid Fire ===
            MakeHeaderField("Aim / Rapid Fire"),
            MakeField("aimMaxDistance", "Aim Max Distance", &Data::aimMaxDistance,
                DragFieldOptions{ .dragSpeed = 1.0f, .minValue = 0.0f, .maxValue = 1000.0f }),
            MakeField("rapidFireInterval", "Rapid Fire Interval", &Data::rapidFireInterval,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.01f, .maxValue = 2.0f }),
            MakeField("rapidFireMoveSpeedMultiplier", "Rapid Fire Move Speed Multiplier",
                &Data::rapidFireMoveSpeedMultiplier,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 2.0f }),
            MakeField("rapidFireJumpPowerMultiplier", "Rapid Fire Jump Power Multiplier",
                &Data::rapidFireJumpPowerMultiplier,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 2.0f }),
            
            // === Rapid Fire Animation ===
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

            // === Slash Burst ===
            MakeHeaderField("Slash Burst"),
            MakeField("slashBurstFireInterval", "Slash Burst Fire Interval", &Data::slashBurstFireInterval,
                DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.01f, .maxValue = 2.0f }),
            MakeField("slashBurstFireDuration", "Slash Burst Fire Duration", &Data::slashBurstFireDuration,
                DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.01f, .maxValue = 2.0f }),
            MakeField("fireTime", "Burst Time", &Data::burstTime,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f }),
            MakeField("inputBufferStartTime", "Input Buffer Start Time", &Data::inputBufferStartTime,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f }),
            MakeField("chainTime", "Chain Time", &Data::chainTime,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f }),
            MakeField("endTime", "End Time", &Data::endTime,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f }),

            // === Slash Burst Effects ===
            MakeHeaderField("Slash Burst Effects"),
            MakeField(
                "slashBurstEffectAssetPath",
                "Slash Burst Effect Asset Path",
                &Data::slashBurstEffectAssetPath),
            MakeStructField(
                "slashBurst1Effect",
                "Slash Burst 1 Effect",
                &Data::slashBurst1Effect,
                GetSlashBurstEffectTransformSettingsSchema(),
                DefaultFieldOptions{}),
            MakeStructField(
                "slashBurst2Effect",
                "Slash Burst 2 Effect",
                &Data::slashBurst2Effect,
                GetSlashBurstEffectTransformSettingsSchema(),
                DefaultFieldOptions{}),
            MakeStructField(
                "slashBurst3LeftEffect",
                "Slash Burst 3 Left Effect",
                &Data::slashBurst3LeftEffect,
                GetSlashBurstEffectTransformSettingsSchema(),
                DefaultFieldOptions{}),
            MakeStructField(
                "slashBurst3RightEffect",
                "Slash Burst 3 Right Effect",
                &Data::slashBurst3RightEffect,
                GetSlashBurstEffectTransformSettingsSchema(),
                DefaultFieldOptions{}),
            MakeField("slashBurstEffectScale", "Slash Burst Effect Scale", &Data::slashBurstEffectScale,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 20.0f }),

            // === Slash Burst Movement ===
            MakeHeaderField("Slash Burst Movement"),
            MakeField("stepMoveDuration", "Step Move Duration", &Data::stepMoveDuration,
                 DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f}),
            MakeField("stepMoveDistance", "Step Move Distance", &Data::stepMoveDistance,
                DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f}),
            MakeField("finalStepMoveDuration", "Final Step Move Duration", &Data::finalStepMoveDuration,
                DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f}),
            MakeField("finalStepMoveDistance", "Final Step Move Distance", &Data::finalStepMoveDistance,
                DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f}),
            MakeField("slashBurstKnockbackDistance", "Knockback Distance", &Data::slashBurstKnockbackDistance,
                DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 50.0f}),
            MakeField("slashBurstMaxKnockbackDistanceFromPlayer", "Max Knockback Distance From Player",
                &Data::slashBurstMaxKnockbackDistanceFromPlayer,
                DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 100.0f}),
            MakeField("slashBurstKnockbackStartMargin", "Knockback Start Margin",
                &Data::slashBurstKnockbackStartMargin,
                DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f}),
            MakeField("slashBurstKnockbackDuration", "Knockback Duration", &Data::slashBurstKnockbackDuration,
                DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.01f, .maxValue = 10.0f}),

                // === Bullet Spread ===
            MakeHeaderField("Bullet Spread"),
            MakeField("bulletSpeed", "Bullet Speed", &Data::bulletSpeed,
                DragFieldOptions{.dragSpeed = 1.0f, .minValue = 0.0f, .maxValue = 1000.0f }),
            MakeField("bulletRadius", "Bullet Radius", &Data::bulletRadius,
                DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f }),
            MakeField("bulletLifetime", "Bullet Lifetime", &Data::bulletLifetime,
                DragFieldOptions{.dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 60.0f }),
            MakeField("bulletSpawnForwardOffset", "Bullet Spawn Forward Offset", &Data::bulletSpawnForwardOffset,
                DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f }),

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

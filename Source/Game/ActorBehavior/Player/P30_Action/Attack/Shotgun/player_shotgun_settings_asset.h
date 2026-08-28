// player_shotgun_settings_asset.h
#pragma once
#include <DirectXMath.h>
#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Editor/Schema/field_editor.h"
#include "Utility/mi_curve.h"

namespace PlayerShotgunSettings {
    struct Data
    {
        // === 基本設定 ===
        float   chargeTime = 1.0f;          // チャージにかかる時間（秒）
        float   recoveryTime = 0.3f;        // リカバリーにかかる時間（秒）

        // === エイム設定 ===
        float   aimFOV = 60.0f;              // エイム時のカメラFOV（度）
        float   aimCameraDistance = 2.0f;    // エイム時のカメラ距離（メートル）
        DirectX::XMFLOAT3 aimCameraLocalOffset = { 1.0f, 0.5f, 0.0f }; // エイム時のカメラローカルオフセット（メートル）
        float   aimTransitionTime = 0.2f;    // エイムへの遷移時間（秒）
        float   aimMoveSpeedMultiplier = 0.7f;   // エイム時の移動速度倍率（0.0f～1.0f）

        // === チャージ設定 ===
        float   chargeFOV = 70.0f;           // チャージ時のカメラFOV（度）
        float   chargeStartDelay = 0.1f;     // チャージ開始時の遅延（秒）
        float   chargeStartTransitionTime = 0.2f; // チャージ開始時の遷移時間（秒）
        float   chargeResetTransitionTime = 0.2f; // チャージリセット時の遷移時間（秒）

        // === 弾丸設定 ===
        float   minBulletSpeed = 25.0f;
        float   maxBulletSpeed = 45.0f;
        float   minBulletRadius = 0.25f;
        float   maxBulletRadius = 0.75f;
        float   bulletLifetime = 3.0f;
        float   bulletSpawnForwardOffset = 0.5f;
        float   aimMaxDistance = 1000.0f;

        // === エフェクト配置設定 ===
        DirectX::XMFLOAT3 chargeEffectOffset = { 1.0f, 1.3f, 0.0f };
        DirectX::XMFLOAT3 chargeCompleteEffectOffset = { 1.0f, 1.3f, 0.0f };
        DirectX::XMFLOAT3 muzzleFlashEffectOffset = { 1.0f, 1.0f, 0.0f };
        float   chargeCompleteEffectScale = 2.5f;

        // === ポストエフェクト設定 ===
        float   aimEnterRadialBlurStrength = 0.5f;
        float   aimExitRadialBlurStrength = 0.3f;
        float   aimRadialBlurHoldTime = 0.01f;
        float   chargeMonoMaskStrength = 0.6f;
    };

    using json = nlohmann::json;
    inline static const auto& GetSchema()
    {
        static const auto& schema = FieldSchema{
            // === Shotgun Base Settings ===
            MakeHeaderField("Shotgun Base Settings"),
            MakeField(
                "chargeTime",
                "Charge Time",
                &Data::chargeTime,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.0f,
                    .maxValue = 10.0f }),
            MakeField(
                "recoveryTime",
                "Recovery Time",
                &Data::recoveryTime,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.0f,
                    .maxValue = 10.0f }),
            // === Shotgun Aim Settings ===
            MakeHeaderField("Shotgun Aim Settings"),
            MakeField(
                "aimFOV",
                "Aim FOV",
                &Data::aimFOV,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 1.0f,
                    .maxValue = 180.0f }),
            MakeField(
                "aimCameraDistance",
                "Aim Camera Distance",
                &Data::aimCameraDistance,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.1f,
                    .maxValue = 10.0f }),
            MakeField(
                "aimCameraLocalOffset",
                "Aim Camera Local Offset",
                &Data::aimCameraLocalOffset,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = -10.0f,
                    .maxValue = 10.0f }),
            MakeField(
                "aimTransitionTime",
                "Aim Transition Time",
                &Data::aimTransitionTime,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 0.0f,
                    .maxValue = 10.0f }),
            MakeField(
                "aimSpeedMultiplier",
                "Aim Move Speed Multiplier",
                &Data::aimMoveSpeedMultiplier,
                DragFieldOptions{
                    .dragSpeed = 0.01f,
                    .minValue = 0.0f,
                    .maxValue = 1.0f }),
            // === Shotgun Charge Settings ===
            MakeHeaderField("Shotgun Charge Settings"),
            MakeField(
                "chargeFOV",
                "Charge FOV",
                &Data::chargeFOV,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = 1.0f,
                    .maxValue = 180.0f }),
            MakeField(
                "chargeStartDelay",
                "Charge Start Delay",
                &Data::chargeStartDelay,
                DragFieldOptions{
                    .dragSpeed = 0.01f,
                    .minValue = 0.0f,
                    .maxValue = 10.0f }),
            MakeField(
                "chargeStartTransitionTime",
                "Charge Start Transition Time",
                &Data::chargeStartTransitionTime,
                DragFieldOptions{
                    .dragSpeed = 0.01f,
                    .minValue = 0.0f,
                    .maxValue = 10.0f }),
            MakeField(
                "chargeResetTransitionTime",
                "Charge Reset Transition Time",
                &Data::chargeResetTransitionTime,
                DragFieldOptions{
                    .dragSpeed = 0.01f,
                    .minValue = 0.0f,
                    .maxValue = 10.0f }),

            // === Shotgun Projectile Settings ===
            MakeHeaderField("Shotgun Projectile Settings"),
            MakeField("minBulletSpeed", "Min Bullet Speed", &Data::minBulletSpeed,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 500.0f }),
            MakeField("maxBulletSpeed", "Max Bullet Speed", &Data::maxBulletSpeed,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 500.0f }),
            MakeField("minBulletRadius", "Min Bullet Radius", &Data::minBulletRadius,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f }),
            MakeField("maxBulletRadius", "Max Bullet Radius", &Data::maxBulletRadius,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f }),
            MakeField("bulletLifetime", "Bullet Lifetime", &Data::bulletLifetime,
                DragFieldOptions{ .dragSpeed = 0.1f, .minValue = 0.0f, .maxValue = 30.0f }),
            MakeField("bulletSpawnForwardOffset", "Bullet Spawn Forward Offset", &Data::bulletSpawnForwardOffset,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f }),
            MakeField("aimMaxDistance", "Aim Max Distance", &Data::aimMaxDistance,
                DragFieldOptions{ .dragSpeed = 1.0f, .minValue = 0.0f, .maxValue = 10000.0f }),

            // === Shotgun Effect Transform Settings ===
            MakeHeaderField("Shotgun Effect Transform Settings"),
            MakeField("chargeEffectOffset", "Charge Effect Offset", &Data::chargeEffectOffset,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = -10.0f, .maxValue = 10.0f }),
            MakeField("chargeCompleteEffectOffset", "Charge Complete Effect Offset", &Data::chargeCompleteEffectOffset,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = -10.0f, .maxValue = 10.0f }),
            MakeField("muzzleFlashEffectOffset", "Muzzle Flash Effect Offset", &Data::muzzleFlashEffectOffset,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = -10.0f, .maxValue = 10.0f }),
            MakeField("chargeCompleteEffectScale", "Charge Complete Effect Scale", &Data::chargeCompleteEffectScale,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 20.0f }),

            // === Shotgun Post Effect Settings ===
            MakeHeaderField("Shotgun Effect Settings"),
            MakeField("aimEnterRadialBlurStrength", "Aim Enter Radial Blur Strength", &Data::aimEnterRadialBlurStrength,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 1.0f }),
            MakeField("aimExitRadialBlurStrength", "Aim Exit Radial Blur Strength", &Data::aimExitRadialBlurStrength,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 1.0f }),
            MakeField("aimRadialBlurHoldTime", "Aim Radial Blur Hold Time", &Data::aimRadialBlurHoldTime,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 10.0f }),
            MakeField("chargeMonoMaskStrength", "Charge Mono Mask Strength", &Data::chargeMonoMaskStrength,
                DragFieldOptions{ .dragSpeed = 0.01f, .minValue = 0.0f, .maxValue = 1.0f }),
        };
        return schema;
    }
}

class PlayerShotgunSettingsAsset : public DataAsset {
private:
    PlayerShotgunSettings::Data m_data;
    static constexpr std::string_view s_assetTypeName = "PlayerShotgunSettingsAsset";
    static constexpr int s_supportedFormatVersion = 0;

public:
    PlayerShotgunSettingsAsset() : DataAsset(
        DataAssetTypeID::getTypeID<PlayerShotgunSettingsAsset>(),
        s_assetTypeName, 
        s_supportedFormatVersion) {}
    ~PlayerShotgunSettingsAsset() override = default;
    const PlayerShotgunSettings::Data& GetData() const { return m_data; }

    std::unique_ptr<DataAsset> CreateDefaultInstance() const override
    {
        auto clone = std::make_unique<PlayerShotgunSettingsAsset>();
        clone->SetHeader(AssetHeader{ s_assetTypeName.data(), s_supportedFormatVersion, "" });
        clone->m_data = PlayerShotgunSettings::Data{};
        return clone;
    }

    nlohmann::json SerializeData() const override
    {
        nlohmann::json jsonData = FieldSerialization::SerializeFields(m_data, PlayerShotgunSettings::GetSchema());
        return jsonData;
    }
    bool DeserializeDataToApply(const nlohmann::json& jsonData) override
    {
        PlayerShotgunSettings::Data loaded = m_data;
        if (!FieldSerialization::DeserializeFields(jsonData, loaded, PlayerShotgunSettings::GetSchema())) {
            return false;
        }
        m_data = loaded;
        return true;
    }
    bool DrawDataOnEditor() override
    {
        bool changed = false;
        changed |= FieldEditor::DrawFields(m_data, PlayerShotgunSettings::GetSchema());
        return changed;
    }
};

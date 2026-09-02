// player_move_settings_asset.h
#pragma once
#include <DirectXMath.h>

#include "Engine/Asset/Schema/field_master.h"

#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"

#include "Engine/Editor/Schema/field_editor.h"

#include "Utility/mi_curve.h"

namespace PlayerMoveSettings {
    struct AttachedEffectSettings
    {
        std::string particleAssetPath = "asset/Particle/run_dust.particle.json";
        XMFLOAT3 positionOffset = { 0.0f, 0.0f, 0.0f };
    };

    inline static const auto& GetAttachedEffectSettingsSchema()
    {
        static const auto& schema = FieldSchema{
            MakeField(
                "particleAssetPath",
                "Particle Asset Path",
                &AttachedEffectSettings::particleAssetPath),
            MakeField(
                "positionOffset",
                "Position Offset",
                &AttachedEffectSettings::positionOffset,
                DragFieldOptions{
                    .dragSpeed = 0.1f,
                    .minValue = -10.0f,
                    .maxValue = 10.0f }),
        };
        return schema;
    }

    /// @brief PlayerMoveの設定値を保持する構造体
    struct Data
    {
        // === 基本設定 ===
        float   moveSpeed = 10.0f;
        float   jumpForce = 10.0f;
        float   airSpeedMultiplier = 0.9f;    // 空中での移動制御の強さ（0.0f～1.0f）

        // === 接地判定設定 ===
        float   groundCheckRadius = 0.35f;
        float   groundCheckDistance = 0.15f;

        // === 回転設定 ===
        float   rotationSpeed = 10.0f;

        // === 平滑化設定 ===
        float   smoothTime = 0.1f;          // 平滑化の時間（秒）
        float   airSmoothTime = 0.2f;       // 空中での平滑化の時間（秒）
        float   stopSmoothTime = 0.05f;     // 停止時の平滑化の時間（秒）
        float   stopAirSmoothTime = 0.1f;   // 空中での停止時の平滑化の時間（秒）

        // === 重力設定 ===
        MiCurve::FloatCurve gravityScale = {
            .keys = {
                MiCurve::FloatCurveKey{ 1.0f, 1.0f },    // 上昇
                MiCurve::FloatCurveKey{ 0.0f, 0.3f },    // 頂上付近
                MiCurve::FloatCurveKey{ -1.0f, 1.5f },   // 下降
            }
        };

        // === 移動エフェクト設定 ===
        AttachedEffectSettings runDustEffect{
            .particleAssetPath = "asset/Particle/run_dust.particle.json",
            .positionOffset = { 0.0f, -0.3f, 0.0f }
        };
    };

    /// @brief PlayerMoveSettingsのFieldSchemaを取得する
    using json = nlohmann::json;
    inline static const auto& GetSchema()
    {
        using MoveSettings = Data;

        static const auto& schema = FieldSchema{
            // === 基本設定 ===
            MakeHeaderField("Basic Settings"),
            MakeField(
                "moveSpeed",
                "Move Speed",
                &MoveSettings::moveSpeed,
                DragFieldOptions{
                    .dragSpeed = 1.0f,
                    .minValue = 0.0f,
                    .maxValue = 100.0f }),
            MakeField(
                "jumpForce",
                "Jump Force",
                &MoveSettings::jumpForce,
                DragFieldOptions{
                    .dragSpeed = 1.0f,
                    .minValue = 0.0f,
                    .maxValue = 100.0f }),
            MakeField(
                "airSpeedMultiplier",
                "Air Speed Multiplier",
                &MoveSettings::airSpeedMultiplier,
                DragFieldOptions{
                    .dragSpeed = 0.01f,
                    .minValue = 0.0f,
                    .maxValue = 1.0f }),

            // === 接地判定設定 ===
            MakeHeaderField("Ground Check Settings"),
            MakeField(
                "groundCheckRadius",
                "Ground Check Radius",
                &MoveSettings::groundCheckRadius,
                DragFieldOptions{
                    .dragSpeed = 0.01f,
                    .minValue = 0.01f,
                    .maxValue = 10.0f }),
            MakeField(
                "groundCheckDistance",
                "Ground Check Distance",
                &MoveSettings::groundCheckDistance,
                DragFieldOptions{
                    .dragSpeed = 0.01f,
                    .minValue = 0.0f,
                    .maxValue = 10.0f }),

            // === 回転設定 ===
            MakeHeaderField("Rotation Settings"),
            MakeField(
                "rotationSpeed",
                "Rotation Speed",
                &MoveSettings::rotationSpeed,
                DragFieldOptions{
                    .dragSpeed = 1.0f,
                    .minValue = 0.0f,
                    .maxValue = 100.0f }),

            // === 平滑化設定 ===
            MakeHeaderField("Smoothing Settings"),
            MakeField(
                "smoothTime",
                "Smooth Time",
                &MoveSettings::smoothTime,
                DragFieldOptions{
                    .dragSpeed = 0.01f,
                    .minValue = 0.01f,
                    .maxValue = 10.0f }),
            MakeField(
                "airSmoothTime",
                "Air Smooth Time",
                &MoveSettings::airSmoothTime,
                DragFieldOptions{
                    .dragSpeed = 0.01f,
                    .minValue = 0.01f,
                    .maxValue = 10.0f }),
            MakeField(
                "stopSmoothTime",
                "Stop Smooth Time",
                &MoveSettings::stopSmoothTime,
                DragFieldOptions{
                    .dragSpeed = 0.01f,
                    .minValue = 0.01f,
                    .maxValue = 10.0f }),
            MakeField(
                "stopAirSmoothTime",
                "Stop Air Smooth Time",
                &MoveSettings::stopAirSmoothTime,
                DragFieldOptions{
                    .dragSpeed = 0.01f,
                    .minValue = 0.01f,
                    .maxValue = 10.0f }),

            // === 重力設定 ===
            MakeHeaderField("Gravity Settings"),
            MakeField(
                "gravityScale",
                "Gravity Scale",
                &MoveSettings::gravityScale,
                DefaultFieldOptions{}
            ),

            // === 移動エフェクト設定 ===
            MakeHeaderField("Effect Settings"),
            MakeStructField(
                "runDustEffect",
                "Run Dust Effect",
                &MoveSettings::runDustEffect,
                GetAttachedEffectSettingsSchema(),
                DefaultFieldOptions{}),
        };
        return schema;
    }
}

class PlayerMoveSettingsAsset : public DataAsset {
private:
    PlayerMoveSettings::Data m_data;

    static constexpr std::string_view s_assetTypeName = "PlayerMoveSettingsAsset";
    static constexpr int s_supportedFormatVersion = 0;

public:
    PlayerMoveSettingsAsset() : DataAsset(
        DataAssetTypeID::getTypeID<PlayerMoveSettingsAsset>(),
        s_assetTypeName, 
        s_supportedFormatVersion) {}
    ~PlayerMoveSettingsAsset() override = default;

    /// @brief PlayerMoveSettingsのデータを取得する
    const PlayerMoveSettings::Data& GetData() const { return m_data; }

    // === DataAssetのオーバーライド関数 ===

    /// @brief PlayerMoveAssetのデフォルトインスタンスを作成する
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override
    {
        auto clone = std::make_unique<PlayerMoveSettingsAsset>();
        clone->SetHeader(AssetHeader{ s_assetTypeName.data(), s_supportedFormatVersion, "" });
        clone->m_data = PlayerMoveSettings::Data{};
        return clone;
    }

    /// @brief PlayerMoveSettingsをJSON形式でシリアライズする
    nlohmann::json SerializeData() const override
    {
        nlohmann::json jsonData = FieldSerialization::SerializeFields(m_data, PlayerMoveSettings::GetSchema());
        return jsonData;
    }

    /// @brief PlayerMoveSettingsをJSON形式からデシリアライズする
    bool DeserializeDataToApply(const nlohmann::json& jsonData) override
    {
        PlayerMoveSettings::Data loaded = m_data;
        if (!FieldSerialization::DeserializeFields(jsonData, loaded, PlayerMoveSettings::GetSchema())) {
            return false;
        }

        // デシリアライズに成功した場合は、実データに反映する
        m_data = loaded;
        return true;
    }

    /// @brief PlayerMoveSettingsのデータをエディター上で描画する
    bool DrawDataOnEditor() override
    {
        bool changed = false;
        changed |= FieldEditor::DrawFields(m_data, PlayerMoveSettings::GetSchema());
        return changed;
    }


};

// player_move_asset.h
#pragma once
#include "Engine/Asset/Schema/field_master.h"

#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"

#include "player_move_context.h"

namespace PlayerMoveAssetsSchema
{
    using json = nlohmann::json;
    // PlayerMoveのスキーマを取得
    inline const auto& GetPlayerMoveSchema()
    {
        using MoveSettings = PlayerMoveSettings;

        static const auto& schema = FieldSchema{
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
                "rotationSpeed",
                "Rotation Speed",
                &MoveSettings::rotationSpeed,
                DragFieldOptions{
                    .dragSpeed = 1.0f,
                    .minValue = 0.0f,
                    .maxValue = 100.0f }),

            MakeField(
                "smoothTime",
                "Smooth Time",
                &MoveSettings::smoothTime,
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
                    .maxValue = 10.0f })
        };
        return schema;
    }
}

class PlayerMoveAsset : public DataAsset {
private:
    PlayerMoveSettings m_moveSettings;

    static constexpr std::string_view s_assetTypeName = "PlayerMoveAsset";
    static constexpr int s_supportedFormatVersion = 1;

public:
    PlayerMoveAsset() : DataAsset(DataAssetTypeID::getTypeID<PlayerMoveAsset>()) {}
    ~PlayerMoveAsset() override = default;

    std::string_view GetAssetTypeName() const override { return s_assetTypeName; }
    int GetSupportedFormatVersion() const override { return s_supportedFormatVersion; }

    nlohmann::json SerializeData() const override
    {
        nlohmann::json jsonData = FieldSerialization::SerializeFields(m_moveSettings, PlayerMoveAssetsSchema::GetPlayerMoveSchema());
        return jsonData;
    }

    bool DeserializeDataToApply(const nlohmann::json& jsonData) override
    {
        PlayerMoveSettings loaded = m_moveSettings;
        if (!FieldSerialization::DeserializeFields(jsonData, loaded, PlayerMoveAssetsSchema::GetPlayerMoveSchema())) {
            return false;
        }

        // デシリアライズに成功した場合は、実データに反映する
        m_moveSettings = loaded;
        return true;
    }

    /// @brief PlayerMoveSettingsを取得する
    const PlayerMoveSettings& GetMoveSettings() const { return m_moveSettings; }
};

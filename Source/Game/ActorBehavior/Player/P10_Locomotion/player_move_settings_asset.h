// player_move_settings_asset.h
#pragma once
#include "Engine/Asset/Schema/field_master.h"

#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"

#include "Engine/Editor/Schema/field_editor.h"

#include "player_move_context.h"

#include "Utility/mi_curve.h"

namespace PlayerMoveSettings {
    /// @brief PlayerMoveの設定値を保持する構造体
    struct Data
    {
        float   moveSpeed = 10.0f;
        float   jumpForce = 10.0f;

        float   rotationSpeed = 10.0f;

        float   smoothTime = 0.1f; // 平滑化の時間（秒）
        float   stopSmoothTime = 0.05f; // 停止時の平滑化の時間（秒）

        MiCurve::FloatCurve gravityScale = {
            .keys = {
                MiCurve::CurveKey{ 1.0f, 1.0f },    // 上昇
                MiCurve::CurveKey{ 0.0f, 0.3f },    // 頂上付近
                MiCurve::CurveKey{ -1.0f, 1.5f },   // 下降
            }
        };
    };

    /// @brief PlayerMoveSettingsのFieldSchemaを取得する
    using json = nlohmann::json;
    inline static const auto& GetSchema()
    {
        using MoveSettings = Data;

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
                    .maxValue = 10.0f }),

            MakeField(
                "gravityScale",
                "Gravity Scale",
                &MoveSettings::gravityScale,
                DefaultFieldOptions{}
            ),
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

    /// @brief PlayerMoveAssetのデフォルトインスタンスを作成する
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override
    {
        auto clone = std::make_unique<PlayerMoveSettingsAsset>();
        clone->SetHeader(AssetHeader{ s_assetTypeName.data(), s_supportedFormatVersion, ""});
        clone->m_data = PlayerMoveSettings::Data {};
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

    /// @brief PlayerMoveSettingsのデータを取得する
    const PlayerMoveSettings::Data& GetData() const { return m_data; }

};

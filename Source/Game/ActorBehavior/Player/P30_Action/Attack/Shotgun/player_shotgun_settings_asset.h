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
        float   fireRate = 0.5f;            // 発射レート（秒）
        float   recoveryTime = 0.3f;        // リカバリーにかかる時間（秒）

        // === エイム設定 ===
        float   aimFOV = 60.0f;              // エイム時のカメラFOV（度）
        float   aimCameraDistance = 2.0f;    // エイム時のカメラ距離（メートル）
        DirectX::XMFLOAT3 aimCameraLocalOffset = { 1.0f, 0.5f, 0.0f }; // エイム時のカメラローカルオフセット（メートル）
        float   aimTransitionTime = 0.2f;    // エイムへの遷移時間（秒）

        // === エフェクト設定 ===
        float   muzzleFlashDuration = 0.1f; // マズルフラッシュの表示時間（秒）
        float   chargeLightIntensity = 5.0f; // チャージライトの強度

    };

    using json = nlohmann::json;
    inline static const auto& GetSchema()
    {
        static const auto& schema = FieldSchema{};
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
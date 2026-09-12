#pragma once

#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Editor/Schema/field_editor.h"

namespace PlayerWeaponSettings {
    struct Data {
        int maxAmmo = 90;
        int shotgunCost = 30;
        int dualPistolsFireCost = 1;
        int dualPistolsSlashBurstCost = 15;
    };

    inline static const auto& GetSchema()
    {
        static const auto& schema = FieldSchema{
            MakeField("maxAmmo", "Max Ammo", &Data::maxAmmo,
                DragFieldOptions{.dragSpeed=1, .minValue=1, .maxValue=10000}),
            MakeField("shotgunCost", "Shotgun Cost", &Data::shotgunCost,
                DragFieldOptions{.dragSpeed=1, .minValue=1, .maxValue=10000}),
            MakeField("dualPistolsFireCost", "Dual Pistols Fire Cost", &Data::dualPistolsFireCost,
                DragFieldOptions{.dragSpeed=1, .minValue=1, .maxValue=10000}),
            MakeField("dualPistolsSlashBurstCost", "Dual Pistols Slash Burst Cost", &Data::dualPistolsSlashBurstCost,
                DragFieldOptions{.dragSpeed=1, .minValue=1, .maxValue=10000})
        };
        return schema;
    }
}

class PlayerWeaponSettingsAsset : public DataAsset {
private:
    PlayerWeaponSettings::Data m_data;

public:
    PlayerWeaponSettingsAsset()
        : DataAsset(
            DataAssetTypeID::getTypeID<PlayerWeaponSettingsAsset>(),
            "PlayerWeaponSettingsAsset",
            0) {}

    std::unique_ptr<DataAsset> CreateDefaultInstance() const override {
        return std::make_unique<PlayerWeaponSettingsAsset>();
    }

    nlohmann::json SerializeData() const override {
        return FieldSerialization::SerializeFields(
            m_data, PlayerWeaponSettings::GetSchema());
    }

    bool DeserializeDataToApply(const nlohmann::json& jsonData) override {
        PlayerWeaponSettings::Data loaded = m_data;
        if (!FieldSerialization::DeserializeFields(
                jsonData, loaded, PlayerWeaponSettings::GetSchema())) {
            return false;
        }
        m_data = loaded;
        return true;
    }

    bool DrawDataOnEditor() override {
        return FieldEditor::DrawFields(
            m_data, PlayerWeaponSettings::GetSchema());
    }

    const PlayerWeaponSettings::Data& GetData() const { return m_data; }
};

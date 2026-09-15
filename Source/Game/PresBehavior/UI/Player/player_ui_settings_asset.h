//---------------------------------------------------
// File  ：_/UI/Player/player_ui_settings_asset.h
// Date  ：2026/09/08
// Author：Miu Kitamura
//---------------------------------------------------
#pragma once
#include "Game/PresBehavior/UI/ui_layout_settings.h"
#include <DirectXMath.h>
#include <cmath>
#include <string>
#include "Engine/Component/ui_chromatic_echo.h"
#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Editor/Schema/field_editor.h"

namespace PlayerUiSettings {
    using namespace UiLayoutSettings;
    struct HealthBarSettings {
        GroupPlacement placement = {{0, 1}, {180, -65}};
        WidgetTransform slider = {{0, 0}, {280, 20}, 0};
        WidgetTransform label = {{0, -22}, {1, 1}, 0};
        WidgetTransform recoveryGauge = {{0, 28}, {280, 12}, 0};
    };
    struct AmmoCountSettings {
        std::string dualPistolsIconPath = "asset/Texture/Ui/dual_pistols_icon.png";
        std::string shotgunIconPath = "asset/Texture/Ui/shotgun_icon.png";
        std::string slashBurstIconPath = "asset/Texture/Ui/slash_icon.png";
        float iconShakeIntensity = 4.0f;
        float iconShakeDuration = 0.18f;
        GroupPlacement placement = {{1, 1}, {-150, -140}};
        WidgetTransform circleGauge = {{0, 0}, {180, 180}, 0};
        WidgetTransform weaponIcon = {{0, -20}, {80, 48}, 0};
        WidgetTransform currentText = {{-25, 35}, {1.2f, 1.2f}, 0};
        WidgetTransform capacityText = {{28, 35}, {0.8f, 0.8f}, 0};
    };
    struct RemainingLifeSettings {
        GroupPlacement placement = {{0, 0}, {150, 70}};
        WidgetTransform gauge = {{0, 0}, {220, 32}, 0};
    };
    struct Data {
        ChromaticEchoSettings chromaticEcho;
        DirectX::XMFLOAT3 color1 = {0.9f, 0.9f, 0.2f};
        DirectX::XMFLOAT3 color2 = {1, 1, 1};
        PerspectiveSettings perspective;
        HealthBarSettings healthBar;
        AmmoCountSettings ammoCount;
        RemainingLifeSettings remainingLife;
    };
    
    inline const auto& GetHealthBarSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("placement", "Group Placement", &HealthBarSettings::placement, GetGroupPlacementSchema(), DefaultFieldOptions{}),
            MakeStructField("slider", "HP Slider", &HealthBarSettings::slider, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("label", "HP Text", &HealthBarSettings::label, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("recoveryGauge", "Recovery Gauge", &HealthBarSettings::recoveryGauge, GetWidgetTransformSchema(), DefaultFieldOptions{})
        };
        return schema;
    }
    inline const auto& GetAmmoCountSchema() {
        static const auto schema = FieldSchema{
            MakeField("dualPistolsIconPath", "Dual Pistols Icon", &AmmoCountSettings::dualPistolsIconPath),
            MakeField("shotgunIconPath", "Shotgun Icon", &AmmoCountSettings::shotgunIconPath),
            MakeField("slashBurstIconPath", "Slash Burst Icon", &AmmoCountSettings::slashBurstIconPath),
            MakeField("iconShakeIntensity", "Icon Shake (px)", &AmmoCountSettings::iconShakeIntensity,
                DragFieldOptions{.dragSpeed=0.1f, .minValue=0, .maxValue=100}),
            MakeField("iconShakeDuration", "Icon Shake Duration (s)", &AmmoCountSettings::iconShakeDuration,
                DragFieldOptions{.dragSpeed=0.01f, .minValue=0, .maxValue=10}),
            MakeStructField("placement", "Group Placement", &AmmoCountSettings::placement, GetGroupPlacementSchema(), DefaultFieldOptions{}),
            MakeStructField("circleGauge", "Circle Gauge", &AmmoCountSettings::circleGauge, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("weaponIcon", "Weapon Icon", &AmmoCountSettings::weaponIcon, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("currentText", "Current Ammo Text", &AmmoCountSettings::currentText, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("capacityText", "Capacity Text", &AmmoCountSettings::capacityText, GetWidgetTransformSchema(), DefaultFieldOptions{})
        };
        return schema;
    }
    inline const auto& GetRemainingLifeSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("placement", "Group Placement", &RemainingLifeSettings::placement, GetGroupPlacementSchema(), DefaultFieldOptions{}),
            MakeStructField("gauge", "Life Gauge", &RemainingLifeSettings::gauge, GetWidgetTransformSchema(), DefaultFieldOptions{})
        };
        return schema;
    }
    inline const auto& GetSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("chromaticEcho", "Chromatic Echo", &Data::chromaticEcho, GetEchoSchema(), DefaultFieldOptions{}),
            MakeField("color1", "Color 1", &Data::color1, ColorFieldOptions{}),
            MakeField("color2", "Color 2", &Data::color2, ColorFieldOptions{}),
            MakeStructField("perspective", "HUD Perspective", &Data::perspective, GetPerspectiveSchema(), DefaultFieldOptions{}),
            MakeStructField("healthBar", "Health Bar", &Data::healthBar, GetHealthBarSchema(), DefaultFieldOptions{}),
            MakeStructField("ammoCount", "Ammo Count", &Data::ammoCount, GetAmmoCountSchema(), DefaultFieldOptions{}),
            MakeStructField("remainingLife", "Remaining Life", &Data::remainingLife, GetRemainingLifeSchema(), DefaultFieldOptions{})
        };
        return schema;
    }


}

class PlayerUiSettingsAsset : public DataAsset {
    PlayerUiSettings::Data m_data;
public:
    PlayerUiSettingsAsset() : DataAsset(DataAssetTypeID::getTypeID<PlayerUiSettingsAsset>(), "PlayerUiSettingsAsset", 0) {}
    const PlayerUiSettings::Data& GetData() const { return m_data; }
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override {
        return std::make_unique<PlayerUiSettingsAsset>();
    }
    nlohmann::json SerializeData() const override {
        return FieldSerialization::SerializeFields(m_data, PlayerUiSettings::GetSchema());
    }
    bool DeserializeDataToApply(const nlohmann::json& jsonData) override {
        auto loaded = m_data;
        if (!FieldSerialization::DeserializeFields(jsonData, loaded, PlayerUiSettings::GetSchema())) return false;
        m_data = loaded;
        return true;
    }
    bool DrawDataOnEditor() override {
        return FieldEditor::DrawFields(m_data, PlayerUiSettings::GetSchema());
    }
};

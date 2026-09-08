#pragma once
#include <DirectXMath.h>
#include <cmath>
#include "Engine/Asset/Schema/field_master.h"
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Editor/Schema/field_editor.h"

namespace PlayerUiSettings {
    struct WidgetTransform {
        DirectX::XMFLOAT2 position = {}; // グループ中心からのピクセル移動量
        DirectX::XMFLOAT2 size = {1, 1}; // 画像・バーはピクセル、文字はXY倍率
        float rotationDegrees = 0.0f;
    };
    struct GroupPlacement {
        DirectX::XMFLOAT2 screenAnchor = {}; // 左上(0,0)、右下(1,1)
        DirectX::XMFLOAT2 position = {};
    };
    struct HealthBarSettings {
        GroupPlacement placement = {{0, 1}, {180, -65}};
        WidgetTransform slider = {{0, 0}, {280, 20}, 0};
        WidgetTransform label = {{0, -22}, {1, 1}, 0};
    };
    struct AmmoCountSettings {
        GroupPlacement placement = {{1, 1}, {-150, -65}};
        WidgetTransform slider = {{0, 0}, {220, 16}, 0};
        WidgetTransform label = {{0, -20}, {1, 1}, 0};
        WidgetTransform fillImage = {{0, -40}, {220, 220}, 0};
        WidgetTransform backgroundImage = {{0, -40}, {220, 220}, 0};
    };
    struct Data {
        HealthBarSettings healthBar;
        AmmoCountSettings ammoCount;
    };

    inline DirectX::XMFLOAT2 ResolveGroupPosition(const GroupPlacement& placement,
        const DirectX::XMFLOAT2& screenSize) {
        return {screenSize.x * placement.screenAnchor.x + placement.position.x,
            screenSize.y * placement.screenAnchor.y + placement.position.y};
    }

    inline const auto& GetWidgetTransformSchema() {
        static const auto schema = FieldSchema{
            MakeField("position", "Position (px)", &WidgetTransform::position,
                DragFieldOptions{.dragSpeed=1, .minValue=-10000, .maxValue=10000}),
            MakeField("size", "Size (px) / Text XY Scale", &WidgetTransform::size,
                DragFieldOptions{.dragSpeed=0.1f, .minValue=0, .maxValue=10000}),
            MakeField("rotationDegrees", "Rotation (degrees)", &WidgetTransform::rotationDegrees,
                DragFieldOptions{.dragSpeed=1, .minValue=-360, .maxValue=360})
        };
        return schema;
    }
    inline const auto& GetGroupPlacementSchema() {
        static const auto schema = FieldSchema{
            MakeField("screenAnchor", "Screen Anchor (0-1)", &GroupPlacement::screenAnchor,
                DragFieldOptions{.dragSpeed=0.01f, .minValue=0, .maxValue=1}),
            MakeField("position", "Position (px)", &GroupPlacement::position,
                DragFieldOptions{.dragSpeed=1, .minValue=-10000, .maxValue=10000})
        };
        return schema;
    }
    inline const auto& GetHealthBarSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("placement", "Group Placement", &HealthBarSettings::placement, GetGroupPlacementSchema(), DefaultFieldOptions{}),
            MakeStructField("slider", "HP Slider", &HealthBarSettings::slider, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("label", "HP Text", &HealthBarSettings::label, GetWidgetTransformSchema(), DefaultFieldOptions{})
        };
        return schema;
    }
    inline const auto& GetAmmoCountSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("placement", "Group Placement", &AmmoCountSettings::placement, GetGroupPlacementSchema(), DefaultFieldOptions{}),
            MakeStructField("slider", "Ammo Slider", &AmmoCountSettings::slider, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("label", "Ammo Text", &AmmoCountSettings::label, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("fillImage", "Fill Image", &AmmoCountSettings::fillImage, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("backgroundImage", "Background Image", &AmmoCountSettings::backgroundImage, GetWidgetTransformSchema(), DefaultFieldOptions{})
        };
        return schema;
    }
    inline const auto& GetSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("healthBar", "Health Bar", &Data::healthBar, GetHealthBarSchema(), DefaultFieldOptions{}),
            MakeStructField("ammoCount", "Ammo Count", &Data::ammoCount, GetAmmoCountSchema(), DefaultFieldOptions{})
        };
        return schema;
    }
    inline bool IsValid(const WidgetTransform& t) {
        return std::isfinite(t.position.x) && std::isfinite(t.position.y)
            && std::isfinite(t.size.x) && std::isfinite(t.size.y) && t.size.x >= 0 && t.size.y >= 0
            && std::isfinite(t.rotationDegrees);
    }
    inline bool IsValid(const GroupPlacement& p) {
        return std::isfinite(p.position.x) && std::isfinite(p.position.y)
            && std::isfinite(p.screenAnchor.x) && std::isfinite(p.screenAnchor.y)
            && p.screenAnchor.x >= 0 && p.screenAnchor.x <= 1
            && p.screenAnchor.y >= 0 && p.screenAnchor.y <= 1;
    }
    inline bool IsValid(const Data& d) {
        return IsValid(d.healthBar.placement) && IsValid(d.healthBar.slider) && IsValid(d.healthBar.label)
            && IsValid(d.ammoCount.placement) && IsValid(d.ammoCount.slider) && IsValid(d.ammoCount.label)
            && IsValid(d.ammoCount.fillImage) && IsValid(d.ammoCount.backgroundImage);
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
        if (!FieldSerialization::DeserializeFields(jsonData, loaded, PlayerUiSettings::GetSchema())
            || !PlayerUiSettings::IsValid(loaded)) return false;
        m_data = loaded;
        return true;
    }
    bool DrawDataOnEditor() override {
        return FieldEditor::DrawFields(m_data, PlayerUiSettings::GetSchema());
    }
};
// player_ui_settings_asset.h
#pragma once
#include <DirectXMath.h>
#include <cmath>
#include "Engine/Component/ui_chromatic_echo.h"
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
    // Marker is derived from the bar, so it has no independent editor fields.
    inline WidgetTransform MakeHealthMarker(const WidgetTransform& bar, float fraction) {
        const float angle = DirectX::XMConvertToRadians(bar.rotationDegrees);
        const float x = bar.size.x * (fraction - 0.5f);
        return {{bar.position.x + x * std::cos(angle), bar.position.y + x * std::sin(angle)},
            {2, bar.size.y + 8}, bar.rotationDegrees};
    }
    struct GroupPlacement {
        DirectX::XMFLOAT2 screenAnchor = {}; // 左上(0,0)、右下(1,1)
        DirectX::XMFLOAT2 position = {};
    };
    struct HealthBarSettings {
        GroupPlacement placement = {{0, 1}, {180, -65}};
        WidgetTransform slider = {{0, 0}, {280, 20}, 0};
        WidgetTransform label = {{0, -22}, {1, 1}, 0};
        WidgetTransform recoveryGauge = {{0, 28}, {280, 12}, 0};
    };
    struct AmmoCountSettings {
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
    struct PerspectiveSettings {
        bool enabled = true;
        DirectX::XMFLOAT2 vanishingPoint = {0.5f, 0.5f};
        float tiltDegrees = 12.0f;
        float maxTiltDegrees = 20.0f;
        float cameraDistance = 1200.0f;
    };
    struct ChromaticEchoSettings {
        bool enabled = false;
        DirectX::XMFLOAT2 center = {0.5f, 0.5f};
        float offsetDistance = 0; // pixels; negative moves toward the center
        float scale = 1.003f;
        DirectX::XMFLOAT3 color = {1, 0, 0};
        float opacity = 0.2f;
        int orderInLayerOffset = -10;
    };

    /// @brief ChromaticEchoSettingsからUiChromaticEchoを計算する関数
    inline UiChromaticEcho ResolveChromaticEcho(
        const ChromaticEchoSettings& settings,
        DirectX::XMFLOAT2 vanishingPoint,
        DirectX::XMFLOAT2 screenAnchor, 
        DirectX::XMFLOAT2 screenSize) 
    {
        UiChromaticEcho echo;
        echo.enabled = settings.enabled;
        echo.center = {settings.center.x * screenSize.x, settings.center.y * screenSize.y};
        echo.scale = settings.scale;
        echo.color = settings.color;
        echo.opacity = settings.opacity;
        echo.orderInLayerOffset = settings.orderInLayerOffset;

        // 消失点からの方向にオフセットを計算する
        const float x = screenAnchor.x - vanishingPoint.x;
        const float y = screenAnchor.y - vanishingPoint.y;

        const float length = std::sqrt(x * x + y * y);
        if (length > 0 && std::isfinite(length) && std::isfinite(settings.offsetDistance)) {
            echo.offset = {x / length * settings.offsetDistance, y / length * settings.offsetDistance};
        }
        return echo;
    }

    struct Data {
        ChromaticEchoSettings chromaticEcho;
        DirectX::XMFLOAT3 color1 = {0.9f, 0.9f, 0.2f};
        DirectX::XMFLOAT3 color2 = {1, 1, 1};
        PerspectiveSettings perspective;
        HealthBarSettings healthBar;
        AmmoCountSettings ammoCount;
        RemainingLifeSettings remainingLife;
    };
    
    /// @brief GroupPlacementのスクリーン上の座標を計算する関数
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
            MakeStructField("label", "HP Text", &HealthBarSettings::label, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("recoveryGauge", "Recovery Gauge", &HealthBarSettings::recoveryGauge, GetWidgetTransformSchema(), DefaultFieldOptions{})
        };
        return schema;
    }
    inline const auto& GetAmmoCountSchema() {
        static const auto schema = FieldSchema{
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
    inline const auto& GetPerspectiveSchema() {
        static const auto schema = FieldSchema{
            MakeField("enabled", "Enabled", &PerspectiveSettings::enabled, DefaultFieldOptions{}),
            MakeField("vanishingPoint", "Vanishing Point (0-1)", &PerspectiveSettings::vanishingPoint,
                DragFieldOptions{.dragSpeed=0.01f, .minValue=0, .maxValue=1}),
            MakeField("tiltDegrees", "Tilt (degrees)", &PerspectiveSettings::tiltDegrees,
                DragFieldOptions{.dragSpeed=0.1f, .minValue=0, .maxValue=45}),
            MakeField("maxTiltDegrees", "Max Tilt (degrees)", &PerspectiveSettings::maxTiltDegrees,
                DragFieldOptions{.dragSpeed=0.1f, .minValue=0, .maxValue=45}),
            MakeField("cameraDistance", "Camera Distance (px)", &PerspectiveSettings::cameraDistance,
                DragFieldOptions{.dragSpeed=10, .minValue=100, .maxValue=10000})
        };
        return schema;
    }
    inline const auto& GetEchoSchema() {
        static const auto schema = FieldSchema{
            MakeField("enabled", "Enabled", &ChromaticEchoSettings::enabled, DefaultFieldOptions{}),
            MakeField("center", "Center (0-1)", &ChromaticEchoSettings::center, DragFieldOptions{.dragSpeed=0.01f, .minValue=0, .maxValue=1}),
            MakeField("offsetDistance", "Offset Distance (px)", &ChromaticEchoSettings::offsetDistance, DragFieldOptions{.dragSpeed=0.1f, .minValue=-1000, .maxValue=1000}),
            MakeField("scale", "Scale", &ChromaticEchoSettings::scale, DragFieldOptions{.dragSpeed=0.001f, .minValue=0.01f, .maxValue=3}),
            MakeField("color", "Color", &ChromaticEchoSettings::color, ColorFieldOptions{}),
            MakeField("opacity", "Opacity", &ChromaticEchoSettings::opacity, DragFieldOptions{.dragSpeed=0.01f, .minValue=0, .maxValue=1}),
            MakeField("orderInLayerOffset", "Order In Layer Offset", &ChromaticEchoSettings::orderInLayerOffset, DragFieldOptions{.dragSpeed=1, .minValue=-10000, .maxValue=10000})
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

    // === パラメータが有効な範囲にあるかをチェックする関数 ===
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
    inline bool IsValidColor(const DirectX::XMFLOAT3& c) {
        return std::isfinite(c.x) && c.x >= 0 && c.x <= 1
            && std::isfinite(c.y) && c.y >= 0 && c.y <= 1
            && std::isfinite(c.z) && c.z >= 0 && c.z <= 1;
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
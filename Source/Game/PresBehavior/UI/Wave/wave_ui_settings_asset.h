#pragma once
#include "Game/PresBehavior/UI/Player/player_ui_settings_asset.h"
#include <algorithm>

namespace WaveUiSettings {
    struct WidgetSettings {
        PlayerUiSettings::GroupPlacement placement = {{0.5f, 0}, {0, 60}};
        PlayerUiSettings::WidgetTransform label = {{0, 0}, {1, 1}, 0};
        int fontSize = 26;
        DirectX::XMFLOAT3 color = {1, 1, 1};
    };
    struct PointsSettings {
        WidgetSettings text;
        PlayerUiSettings::WidgetTransform gauge = {{0, 30}, {320, 10}, 0};
        DirectX::XMFLOAT3 gaugeColor = {0.9f, 0.9f, 0.2f};
        float gaugeSmoothTime = 0.2f;
    };
    struct Data {
        WidgetSettings number;
        PointsSettings points;
        WidgetSettings phase;
        float pulseDuration = 0.3f;
        float pulseScale = 1.15f;
        bool popupEnabled = true;
        float popupDuration = 0.8f;
        float popupHeightOffset = 3;
        float popupRiseDistance = 1.5f;
        float popupScale = 1.5f;
        DirectX::XMFLOAT3 popupColor = {1, 0.9f, 0.2f};
        Data() {
            points.text.placement.position.y = 100;
            points.text.fontSize = 22;
            phase.placement.position.y = 165;
            phase.fontSize = 24;
        }
    };
    inline const auto& GetWidgetSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("placement", "Group Placement", &WidgetSettings::placement, PlayerUiSettings::GetGroupPlacementSchema(), DefaultFieldOptions{}),
            MakeStructField("label", "Text Transform", &WidgetSettings::label, PlayerUiSettings::GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeField("fontSize", "Font Size", &WidgetSettings::fontSize, DragFieldOptions{.dragSpeed = 1, .minValue = 1, .maxValue = 128}),
            MakeField("color", "Color", &WidgetSettings::color, ColorFieldOptions{})
        };
        return schema;
    }
    inline const auto& GetPointsSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("text", "Text", &PointsSettings::text, GetWidgetSchema(), DefaultFieldOptions{}),
            MakeStructField("gauge", "Gauge Transform", &PointsSettings::gauge, PlayerUiSettings::GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeField("gaugeSmoothTime", "Gauge Smooth Time (s)", &PointsSettings::gaugeSmoothTime, DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.01f, .maxValue = 5}),
            MakeField("gaugeColor", "Gauge Color", &PointsSettings::gaugeColor, ColorFieldOptions{})
        };
        return schema;
    }
    inline const auto& GetSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("number", "Wave Number", &Data::number, GetWidgetSchema(), DefaultFieldOptions{}),
            MakeStructField("points", "Points", &Data::points, GetPointsSchema(), DefaultFieldOptions{}),
            MakeStructField("phase", "Phase", &Data::phase, GetWidgetSchema(), DefaultFieldOptions{}),
            MakeField("pulseDuration", "Pulse Duration", &Data::pulseDuration, DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0, .maxValue = 10}),
            MakeField("pulseScale", "Pulse Scale", &Data::pulseScale, DragFieldOptions{.dragSpeed = 0.01f, .minValue = 1, .maxValue = 3}),
            MakeField("popupEnabled", "Defeat Popup", &Data::popupEnabled),
            MakeField("popupDuration", "Popup Duration", &Data::popupDuration, DragFieldOptions{.dragSpeed = 0.01f, .minValue = 0.01f, .maxValue = 10}),
            MakeField("popupHeightOffset", "Popup Height Offset (world)", &Data::popupHeightOffset, DragFieldOptions{.dragSpeed = 0.1f, .minValue = -10, .maxValue = 20}),
            MakeField("popupRiseDistance", "Popup Rise Distance (world)", &Data::popupRiseDistance, DragFieldOptions{.dragSpeed = 0.1f, .minValue = 0, .maxValue = 20}),
            MakeField("popupScale", "Popup Scale", &Data::popupScale, DragFieldOptions{.dragSpeed = 0.1f, .minValue = 0.1f, .maxValue = 10}),
            MakeField("popupColor", "Popup Color", &Data::popupColor, ColorFieldOptions{})
        };
        return schema;
    }
    inline void Sanitize(Data& data) {
        auto finite = [](float value, float fallback) { return std::isfinite(value) ? value : fallback; };
        data.points.gaugeSmoothTime = std::clamp(finite(data.points.gaugeSmoothTime, 0.2f), 0.01f, 5.0f);
        data.pulseDuration = (std::max)(0.0f, finite(data.pulseDuration, 0.3f));
        data.pulseScale = std::clamp(finite(data.pulseScale, 1.15f), 1.0f, 3.0f);
        data.popupDuration = (std::max)(0.01f, finite(data.popupDuration, 0.8f));
        data.popupHeightOffset = finite(data.popupHeightOffset, 3);
        data.popupRiseDistance = (std::max)(0.0f, finite(data.popupRiseDistance, 1.5f));
        data.popupScale = (std::max)(0.1f, finite(data.popupScale, 1.5f));
        for (auto* widget : {&data.number, &data.points.text, &data.phase}) widget->fontSize = std::clamp(widget->fontSize, 1, 128);
    }
}
class WaveUiSettingsAsset : public DataAsset {
    WaveUiSettings::Data m_data;
public:
    WaveUiSettingsAsset() : DataAsset(DataAssetTypeID::getTypeID<WaveUiSettingsAsset>(), "WaveUiSettingsAsset", 0) {}
    const WaveUiSettings::Data& GetData() const { return m_data; }
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override { return std::make_unique<WaveUiSettingsAsset>(); }
    nlohmann::json SerializeData() const override { return FieldSerialization::SerializeFields(m_data, WaveUiSettings::GetSchema()); }
    bool DeserializeDataToApply(const nlohmann::json& json) override {
        auto data = m_data;
        if (!FieldSerialization::DeserializeFields(json, data, WaveUiSettings::GetSchema())) return false;
        WaveUiSettings::Sanitize(data); m_data = data; return true;
    }
    bool DrawDataOnEditor() override {
        const bool changed = FieldEditor::DrawFields(m_data, WaveUiSettings::GetSchema());
        if (changed) WaveUiSettings::Sanitize(m_data);
        return changed;
    }
};

#pragma once
#include "Game/PresBehavior/UI/ui_layout_settings.h"
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Editor/Schema/field_editor.h"
#include <string>

namespace GamePauseUiSettings {
    using namespace UiLayoutSettings;

    struct Data {
        GroupPlacement placement = {{0.5f, 0.5f}, {0, 0}};
        WidgetTransform panel = {{0, 0}, {640, 260}, 0};
        WidgetTransform message = {{0, -50}, {1, 1}, 0};
        WidgetTransform yes = {{-130, 65}, {1, 1}, 0};
        WidgetTransform no = {{130, 65}, {1, 1}, 0};
        WidgetTransform selection = {{0, 0}, {180, 52}, 0};
        std::string panelImagePath = "asset/Texture/white.bmp";
        std::string selectionImagePath = "asset/Texture/white.bmp";
        std::string messageText = "タイトルへ戻りますか？";
        std::string yesText = "はい";
        std::string noText = "いいえ";
        DirectX::XMFLOAT3 textColor = {1, 1, 1};
        DirectX::XMFLOAT3 selectedTextColor = {1, 1, 0.5f};
        DirectX::XMFLOAT3 panelColor = {0.08f, 0.1f, 0.15f};
        DirectX::XMFLOAT3 selectionColor = {0.2f, 0.35f, 0.55f};
        float dimmerOpacity = 0.65f;
        float selectionOpacity = 0.8f;
        float selectionMoveDuration = 0.15f;
        int fontSize = 32;
        bool applyPerspective = false;
        bool applyChromaticEcho = false;
        PerspectiveSettings perspective;
        ChromaticEchoSettings chromaticEcho;
    };

    inline const auto& GetSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("placement", "Placement", &Data::placement, GetGroupPlacementSchema(), DefaultFieldOptions{}),
            MakeStructField("panel", "Panel", &Data::panel, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("message", "Message", &Data::message, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("yes", "Yes", &Data::yes, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("no", "No", &Data::no, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("selection", "Selection", &Data::selection, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeField("panelImagePath", "Panel Image", &Data::panelImagePath),
            MakeField("selectionImagePath", "Selection Image", &Data::selectionImagePath),
            MakeField("messageText", "Message Text", &Data::messageText),
            MakeField("yesText", "Yes Text", &Data::yesText),
            MakeField("noText", "No Text", &Data::noText),
            MakeField("textColor", "Text Color", &Data::textColor, ColorFieldOptions{}),
            MakeField("selectedTextColor", "Selected Text Color", &Data::selectedTextColor, ColorFieldOptions{}),
            MakeField("panelColor", "Panel Color", &Data::panelColor, ColorFieldOptions{}),
            MakeField("selectionColor", "Selection Color", &Data::selectionColor, ColorFieldOptions{}),
            MakeField("dimmerOpacity", "Dimmer Opacity", &Data::dimmerOpacity, DragFieldOptions{.dragSpeed=.01f,.minValue=0,.maxValue=1}),
            MakeField("selectionOpacity", "Selection Opacity", &Data::selectionOpacity, DragFieldOptions{.dragSpeed=.01f,.minValue=0,.maxValue=1}),
            MakeField("selectionMoveDuration", "Selection Move Duration", &Data::selectionMoveDuration, DragFieldOptions{.dragSpeed=.01f,.minValue=0,.maxValue=2}),
            MakeField("fontSize", "Font Size", &Data::fontSize, DragFieldOptions{.dragSpeed=1,.minValue=1,.maxValue=256}),
            MakeField("applyPerspective", "Apply Perspective", &Data::applyPerspective),
            MakeField("applyChromaticEcho", "Apply Chromatic Echo", &Data::applyChromaticEcho),
            MakeStructField("perspective", "Perspective", &Data::perspective, GetPerspectiveSchema(), DefaultFieldOptions{}),
            MakeStructField("chromaticEcho", "Chromatic Echo", &Data::chromaticEcho, GetEchoSchema(), DefaultFieldOptions{})
        };
        return schema;
    }
}

class GamePauseUiSettingsAsset : public DataAsset {
    GamePauseUiSettings::Data m_data;
public:
    GamePauseUiSettingsAsset() : DataAsset(DataAssetTypeID::getTypeID<GamePauseUiSettingsAsset>(), "GamePauseUiSettingsAsset", 0) {}
    const GamePauseUiSettings::Data& GetData() const { return m_data; }
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override { return std::make_unique<GamePauseUiSettingsAsset>(); }
    nlohmann::json SerializeData() const override { return FieldSerialization::SerializeFields(m_data, GamePauseUiSettings::GetSchema()); }
    bool DeserializeDataToApply(const nlohmann::json& json) override {
        auto loaded = m_data;
        if (!FieldSerialization::DeserializeFields(json, loaded, GamePauseUiSettings::GetSchema())) return false;
        m_data = std::move(loaded); return true;
    }
    bool DrawDataOnEditor() override { return FieldEditor::DrawFields(m_data, GamePauseUiSettings::GetSchema()); }
};

#pragma once
#include "Game/PresBehavior/UI/ui_layout_settings.h"
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Editor/Schema/field_editor.h"
#include <string>

namespace TitleUiSettings {
    using namespace UiLayoutSettings;
    struct GroupSettings {
        GroupPlacement placement;
        bool applyPerspective = true;
        bool applyChromaticEcho = true;
    };
    struct MenuSettings {
        GroupSettings group = {{{0.75f, 0.55f}, {0, 0}}, true, true};
        WidgetTransform practice = {{0, -70}, {1, 1}, 0};
        WidgetTransform startGame = {{0, 0}, {1, 1}, 0};
        WidgetTransform exit = {{0, 70}, {1, 1}, 0};
        WidgetTransform selection = {{0, 0}, {360, 56}, 0};
        std::string selectionImagePath = "asset/Texture/white.bmp";
        std::string practiceText = "練習場";
        std::string startGameText = "討伐開始";
        std::string exitText = "終了";
    };
    struct PopupSettings {
        GroupSettings group = {{{0.5f, 0.5f}, {0, 0}}, false, false};
        WidgetTransform panel = {{0, 0}, {640, 260}, 0};
        WidgetTransform message = {{0, -50}, {1, 1}, 0};
        WidgetTransform yes = {{-130, 65}, {1, 1}, 0};
        WidgetTransform no = {{130, 65}, {1, 1}, 0};
        WidgetTransform selection = {{0, 0}, {180, 52}, 0};
        std::string panelImagePath = "asset/Texture/white.bmp";
        std::string selectionImagePath = "asset/Texture/white.bmp";
        std::string messageText = "ゲームを終了しますか？";
        std::string yesText = "はい";
        std::string noText = "いいえ";
        float dimmerOpacity = 0.65f;
    };
    struct TextSettings {
        GroupSettings group;
        WidgetTransform text = {{0, 0}, {1, 1}, 0};
        std::string value;
    };
    struct Data {
        MenuSettings menu;
        PopupSettings popup;
        TextSettings version = {{{{1, 1}, {-130, -35}}, false, false}, {{0, 0}, {0.6f, 0.6f}, 0}, "Ver. 0.1.0"};
        TextSettings highScore = {{{{0.75f, 0.32f}, {0, 0}}, true, true}, {{0, 0}, {0.8f, 0.8f}, 0}, "HIGH SCORE"};
        PerspectiveSettings perspective;
        ChromaticEchoSettings chromaticEcho;
        DirectX::XMFLOAT3 textColor = {1, 1, 1};
        DirectX::XMFLOAT3 selectedTextColor = {1, 1, 0.5f};
        DirectX::XMFLOAT3 selectionColor = {0.2f, 0.35f, 0.55f};
        DirectX::XMFLOAT3 panelColor = {0.08f, 0.1f, 0.15f};
        float selectionOpacity = 0.8f;
        float selectionMoveDuration = 0.15f;
        int fontSize = 32;
    };
    inline const auto& GetGroupSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("placement", "Placement", &GroupSettings::placement, GetGroupPlacementSchema(), DefaultFieldOptions{}),
            MakeField("applyPerspective", "Apply Perspective", &GroupSettings::applyPerspective),
            MakeField("applyChromaticEcho", "Apply Chromatic Echo", &GroupSettings::applyChromaticEcho)
        }; return schema;
    }
    inline const auto& GetMenuSettingsSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("group", "Group", &MenuSettings::group, GetGroupSchema(), DefaultFieldOptions{}),
            MakeStructField("practice", "practice", &MenuSettings::practice, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("startGame", "startGame", &MenuSettings::startGame, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("exit", "exit", &MenuSettings::exit, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("selection", "selection", &MenuSettings::selection, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeField("selectionImagePath", "selectionImagePath", &MenuSettings::selectionImagePath),
            MakeField("practiceText", "practiceText", &MenuSettings::practiceText),
            MakeField("startGameText", "startGameText", &MenuSettings::startGameText),
            MakeField("exitText", "exitText", &MenuSettings::exitText)
        }; return schema;
    }
    inline const auto& GetPopupSettingsSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("group", "Group", &PopupSettings::group, GetGroupSchema(), DefaultFieldOptions{}),
            MakeStructField("panel", "panel", &PopupSettings::panel, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("message", "message", &PopupSettings::message, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("yes", "yes", &PopupSettings::yes, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("no", "no", &PopupSettings::no, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("selection", "selection", &PopupSettings::selection, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeField("panelImagePath", "panelImagePath", &PopupSettings::panelImagePath),
            MakeField("selectionImagePath", "selectionImagePath", &PopupSettings::selectionImagePath),
            MakeField("messageText", "messageText", &PopupSettings::messageText),
            MakeField("yesText", "yesText", &PopupSettings::yesText),
            MakeField("noText", "noText", &PopupSettings::noText),
            MakeField("dimmerOpacity", "dimmerOpacity", &PopupSettings::dimmerOpacity, DragFieldOptions{.dragSpeed=0.01f, .minValue=0, .maxValue=1})
        }; return schema;
    }
    inline const auto& GetTextSettingsSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("group", "Group", &TextSettings::group, GetGroupSchema(), DefaultFieldOptions{}),
            MakeStructField("text", "text", &TextSettings::text, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeField("value", "value", &TextSettings::value)
        }; return schema;
    }
    inline const auto& GetSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("menu", "menu", &Data::menu, GetMenuSettingsSchema(), DefaultFieldOptions{}),
            MakeStructField("popup", "popup", &Data::popup, GetPopupSettingsSchema(), DefaultFieldOptions{}),
            MakeStructField("version", "version", &Data::version, GetTextSettingsSchema(), DefaultFieldOptions{}),
            MakeStructField("highScore", "highScore", &Data::highScore, GetTextSettingsSchema(), DefaultFieldOptions{}),
            MakeStructField("perspective", "perspective", &Data::perspective, GetPerspectiveSchema(), DefaultFieldOptions{}),
            MakeStructField("chromaticEcho", "chromaticEcho", &Data::chromaticEcho, GetEchoSchema(), DefaultFieldOptions{}),
            MakeField("textColor", "textColor", &Data::textColor, ColorFieldOptions{}),
            MakeField("selectedTextColor", "selectedTextColor", &Data::selectedTextColor, ColorFieldOptions{}),
            MakeField("selectionColor", "selectionColor", &Data::selectionColor, ColorFieldOptions{}),
            MakeField("panelColor", "panelColor", &Data::panelColor, ColorFieldOptions{}),
            MakeField("selectionOpacity", "Selection Opacity", &Data::selectionOpacity, DragFieldOptions{.dragSpeed=0.01f, .minValue=0, .maxValue=1}),
            MakeField("selectionMoveDuration", "Selection Move Duration", &Data::selectionMoveDuration, DragFieldOptions{.dragSpeed=0.01f, .minValue=0, .maxValue=10}),
            MakeField("fontSize", "Font Size", &Data::fontSize, DragFieldOptions{.dragSpeed=1, .minValue=1, .maxValue=256})
        }; return schema;
    }
}

class TitleUiSettingsAsset : public DataAsset {
    TitleUiSettings::Data m_data;
public:
    TitleUiSettingsAsset() : DataAsset(DataAssetTypeID::getTypeID<TitleUiSettingsAsset>(), "TitleUiSettingsAsset", 0) {}
    const TitleUiSettings::Data& GetData() const { return m_data; }
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override { return std::make_unique<TitleUiSettingsAsset>(); }
    nlohmann::json SerializeData() const override { return FieldSerialization::SerializeFields(m_data, TitleUiSettings::GetSchema()); }
    bool DeserializeDataToApply(const nlohmann::json& json) override {
        auto loaded = m_data;
        if (!FieldSerialization::DeserializeFields(json, loaded, TitleUiSettings::GetSchema())) return false;
        m_data = loaded; return true;
    }
    bool DrawDataOnEditor() override { return FieldEditor::DrawFields(m_data, TitleUiSettings::GetSchema()); }
};

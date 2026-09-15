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
    struct PracticeGuideRowSettings {
        WidgetTransform key;
        WidgetTransform description;
        std::string keyText;
        std::string descriptionText;
    };
    struct PracticeGuideSettings {
        GroupSettings basicGroup = {{{0.16f, 0.5f}, {0, 0}}, true, true};
        GroupSettings combatGroup = {{{0.79f, 0.5f}, {0, 0}}, true, true};
        WidgetTransform basicPanel = {{0, 0}, {430, 350}, 0};
        WidgetTransform basicTitle = {{0, -125}, {1, 1}, 0};
        WidgetTransform combatPanel = {{0, 0}, {700, 480}, 0};
        WidgetTransform combatTitle = {{0, -190}, {1, 1}, 0};
        PracticeGuideRowSettings move = {{{-100, -60}, {0.7f, 0.7f}, 0}, {{65, -60}, {0.7f, 0.7f}, 0}, "WASD", "移動"};
        PracticeGuideRowSettings jump = { {{-100, 0}, {0.7f, 0.7f}, 0}, {{65, 0}, {0.7f, 0.7f}, 0}, "SPACE", "ジャンプ" };
        PracticeGuideRowSettings camera = {{{-100, 10}, {0.7f, 0.7f}, 0}, {{65, 10}, {0.7f, 0.7f}, 0}, "MOUSE", "カメラ操作"};
        PracticeGuideRowSettings returnToTitle = {{{-100, 80}, {0.7f, 0.7f}, 0}, {{65, 80}, {0.7f, 0.7f}, 0}, "BACK SPACE", "タイトルへ戻る"};
        PracticeGuideRowSettings slash = {{{-150, -120}, {0.65f, 0.65f}, 0}, {{190, -120}, {0.65f, 0.65f}, 0}, "左クリック短押し", "斬撃"};
        PracticeGuideRowSettings dualPistols = {{{-150, -60}, {0.65f, 0.65f}, 0}, {{190, -60}, {0.65f, 0.65f}, 0}, "左クリック長押し", "二丁拳銃"};
        PracticeGuideRowSettings aim = {{{-150, 0}, {0.65f, 0.65f}, 0}, {{190, 0}, {0.65f, 0.65f}, 0}, "右クリック長押し", "エイム"};
        PracticeGuideRowSettings shotgun = {{{-150, 60}, {0.65f, 0.65f}, 0}, {{190, 60}, {0.65f, 0.65f}, 0}, "エイム中に左クリック短押し", "ショットガン"};
        PracticeGuideRowSettings shotgunCharge = {{{-150, 120}, {0.65f, 0.65f}, 0}, {{190, 120}, {0.65f, 0.65f}, 0}, "エイム中に左クリック長押し", "ショットガンチャージ"};
        std::string panelImagePath = "asset/Texture/white.bmp";
        std::string basicTitleText = "基本操作";
        std::string combatTitleText = "攻撃操作";
        DirectX::XMFLOAT3 panelColor = {0.08f, 0.1f, 0.15f};
        DirectX::XMFLOAT3 titleColor = {1, 1, 1};
        DirectX::XMFLOAT3 keyColor = {1, 0.93f, 0.32f};
        DirectX::XMFLOAT3 descriptionColor = {1, 1, 1};
        float panelOpacity = 0.8f;
        int titleFontSize = 36;
        int rowFontSize = 24;
    };
    struct Data {
        MenuSettings menu;
        PopupSettings popup;
        PracticeGuideSettings practiceGuide;
        TextSettings version = {{{{1, 1}, {-130, -35}}, false, false}, {{0, 0}, {0.6f, 0.6f}, 0}, "Ver. 0.1.0"};
        TextSettings selectGuide = { {{{0.5f, 0.9f}, {0, 0}}, true, true}, {{0, 0}, {0.8f, 0.8f}, 0}, "矢印キーで選択 / Enterで決定" };
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
    inline const auto& GetPracticeGuideRowSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("key", "Key", &PracticeGuideRowSettings::key, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("description", "Description", &PracticeGuideRowSettings::description, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeField("keyText", "Key Text", &PracticeGuideRowSettings::keyText),
            MakeField("descriptionText", "Description Text", &PracticeGuideRowSettings::descriptionText)
        }; return schema;
    }
    inline const auto& GetPracticeGuideSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("basicGroup", "Basic Group", &PracticeGuideSettings::basicGroup, GetGroupSchema(), DefaultFieldOptions{}),
            MakeStructField("combatGroup", "Combat Group", &PracticeGuideSettings::combatGroup, GetGroupSchema(), DefaultFieldOptions{}),
            MakeStructField("basicPanel", "Basic Panel", &PracticeGuideSettings::basicPanel, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("basicTitle", "Basic Title", &PracticeGuideSettings::basicTitle, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("combatPanel", "Combat Panel", &PracticeGuideSettings::combatPanel, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("combatTitle", "Combat Title", &PracticeGuideSettings::combatTitle, GetWidgetTransformSchema(), DefaultFieldOptions{}),
            MakeStructField("move", "Move", &PracticeGuideSettings::move, GetPracticeGuideRowSchema(), DefaultFieldOptions{}),
            MakeStructField("jump", "Jump", &PracticeGuideSettings::jump, GetPracticeGuideRowSchema(), DefaultFieldOptions{}),
            MakeStructField("camera", "Camera", &PracticeGuideSettings::camera, GetPracticeGuideRowSchema(), DefaultFieldOptions{}),
            MakeStructField("returnToTitle", "Return To Title", &PracticeGuideSettings::returnToTitle, GetPracticeGuideRowSchema(), DefaultFieldOptions{}),
            MakeStructField("slash", "Slash", &PracticeGuideSettings::slash, GetPracticeGuideRowSchema(), DefaultFieldOptions{}),
            MakeStructField("dualPistols", "Dual Pistols", &PracticeGuideSettings::dualPistols, GetPracticeGuideRowSchema(), DefaultFieldOptions{}),
            MakeStructField("aim", "Aim", &PracticeGuideSettings::aim, GetPracticeGuideRowSchema(), DefaultFieldOptions{}),
            MakeStructField("shotgun", "Shotgun", &PracticeGuideSettings::shotgun, GetPracticeGuideRowSchema(), DefaultFieldOptions{}),
            MakeStructField("shotgunCharge", "Shotgun Charge", &PracticeGuideSettings::shotgunCharge, GetPracticeGuideRowSchema(), DefaultFieldOptions{}),
            MakeField("panelImagePath", "Panel Image Path", &PracticeGuideSettings::panelImagePath),
            MakeField("basicTitleText", "Basic Title Text", &PracticeGuideSettings::basicTitleText),
            MakeField("combatTitleText", "Combat Title Text", &PracticeGuideSettings::combatTitleText),
            MakeField("panelColor", "Panel Color", &PracticeGuideSettings::panelColor, ColorFieldOptions{}),
            MakeField("titleColor", "Title Color", &PracticeGuideSettings::titleColor, ColorFieldOptions{}),
            MakeField("keyColor", "Key Color", &PracticeGuideSettings::keyColor, ColorFieldOptions{}),
            MakeField("descriptionColor", "Description Color", &PracticeGuideSettings::descriptionColor, ColorFieldOptions{}),
            MakeField("panelOpacity", "Panel Opacity", &PracticeGuideSettings::panelOpacity, DragFieldOptions{.dragSpeed=0.01f, .minValue=0, .maxValue=1}),
            MakeField("titleFontSize", "Title Font Size", &PracticeGuideSettings::titleFontSize, DragFieldOptions{.dragSpeed=1, .minValue=1, .maxValue=256}),
            MakeField("rowFontSize", "Row Font Size", &PracticeGuideSettings::rowFontSize, DragFieldOptions{.dragSpeed=1, .minValue=1, .maxValue=256})
        }; return schema;
    }
    inline const auto& GetSchema() {
        static const auto schema = FieldSchema{
            MakeStructField("menu", "menu", &Data::menu, GetMenuSettingsSchema(), DefaultFieldOptions{}),
            MakeStructField("popup", "popup", &Data::popup, GetPopupSettingsSchema(), DefaultFieldOptions{}),
            MakeStructField("practiceGuide", "Practice Guide", &Data::practiceGuide, GetPracticeGuideSchema(), DefaultFieldOptions{}),
            MakeStructField("version", "version", &Data::version, GetTextSettingsSchema(), DefaultFieldOptions{}),
            MakeStructField("selectGuide", "selectGuide", &Data::selectGuide, GetTextSettingsSchema(), DefaultFieldOptions{}),
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

#pragma once
#include "Game/PresBehavior/UI/ui_layout_settings.h"
#include "Engine/Asset/DataAsset/data_asset.h"
#include "Engine/Asset/DataAsset/data_asset_type_id.h"
#include "Engine/Editor/Schema/field_editor.h"
namespace ResultUiSettings {
struct Data {
 DirectX::XMFLOAT4 gaugeBgColor = {0.08f,0.1f,0.14f,1.0f};
 DirectX::XMFLOAT4 gaugeFillColor = {0.5f,0.8f,1.0f,1.0f};
 UiLayoutSettings::GroupPlacement wavePlacement = {{0.68f,0.40f},{0,0}};
 UiLayoutSettings::GroupPlacement scorePlacement = {{0.68f,0.70f},{0,0}}, rankPlacement = {{0.68f,0.87f},{0,0}}, menuPlacement = {{0.68f,0.95f},{0,0}};
 UiLayoutSettings::WidgetTransform title = {{-160,0},{0.8f,0.8f},0}, retry = {{160,0},{0.8f,0.8f},0}, selection = {{0,0},{300,52},0};
 DirectX::XMFLOAT3 selectionColor = {0.2f,0.35f,0.55f};
 float selectionOpacity = 0.8f, selectionMoveDuration = 0.15f;
 UiLayoutSettings::PerspectiveSettings perspective;
 UiLayoutSettings::ChromaticEchoSettings chromaticEcho;
 UiLayoutSettings::WidgetTransform gauge = {{0,0},{360,20},0}, label = {{-280,0},{0.65f,0.65f},0}, status = {{260,0},{0.65f,0.65f},0}, summary = {{0,0},{0.8f,0.8f},0};
 DirectX::XMFLOAT3 textColor = {1,1,1}, clearColor = {0.5f,1,0.7f}, failColor = {1,0.5f,0.5f}, selectedColor = {1,1,0.5f};
 float rowSpacing = 55, smoothTime = 0.35f, statusWait = 0.5f, countDuration = 1.2f, rankWait = 0.8f;
 int fontSize = 32;
 float shakeAmplitude = 3.0f, shakeDuration = 0.25f, shakeFrequency = 24.0f;
 bool applyPerspective = true, applyChromaticEcho = true;
};
inline const auto& GetSchema() {
 using namespace UiLayoutSettings;
 static const auto schema = FieldSchema{
 MakeField("shakeAmplitude","Shake Amplitude (px)",&Data::shakeAmplitude,DragFieldOptions{.dragSpeed=0.1f,.minValue=0,.maxValue=100}),
 MakeField("shakeDuration","Shake Duration (s)",&Data::shakeDuration,DragFieldOptions{.dragSpeed=0.01f,.minValue=0,.maxValue=10}),
 MakeField("shakeFrequency","Shake Frequency (Hz)",&Data::shakeFrequency,DragFieldOptions{.dragSpeed=0.1f,.minValue=0,.maxValue=100}),
 MakeField("gaugeBgColor","Gauge Bg Color",&Data::gaugeBgColor,ColorFieldOptions{}),
 MakeField("gaugeFillColor","Gauge Fill Color",&Data::gaugeFillColor,ColorFieldOptions{}),
 MakeStructField("wavePlacement","wavePlacement",&Data::wavePlacement,GetGroupPlacementSchema(),DefaultFieldOptions{}),
 MakeStructField("scorePlacement","scorePlacement",&Data::scorePlacement,GetGroupPlacementSchema(),DefaultFieldOptions{}),
 MakeStructField("rankPlacement","rankPlacement",&Data::rankPlacement,GetGroupPlacementSchema(),DefaultFieldOptions{}),
 MakeStructField("menuPlacement","menuPlacement",&Data::menuPlacement,GetGroupPlacementSchema(),DefaultFieldOptions{}),
 MakeStructField("title","title",&Data::title,GetWidgetTransformSchema(),DefaultFieldOptions{}),
 MakeStructField("retry","retry",&Data::retry,GetWidgetTransformSchema(),DefaultFieldOptions{}),
 MakeStructField("selection","selection",&Data::selection,GetWidgetTransformSchema(),DefaultFieldOptions{}),
 MakeField("selectionColor","Selection Color",&Data::selectionColor,ColorFieldOptions{}),
 MakeField("selectionOpacity","Selection Opacity",&Data::selectionOpacity,DragFieldOptions{.dragSpeed=0.01f,.minValue=0,.maxValue=1}),
 MakeField("selectionMoveDuration","Selection Move Duration",&Data::selectionMoveDuration,DragFieldOptions{.dragSpeed=0.01f,.minValue=0,.maxValue=10}),
 MakeStructField("perspective","perspective",&Data::perspective,GetPerspectiveSchema(),DefaultFieldOptions{}),
 MakeStructField("chromaticEcho","chromaticEcho",&Data::chromaticEcho,GetEchoSchema(),DefaultFieldOptions{}),
 MakeStructField("gauge","gauge",&Data::gauge,GetWidgetTransformSchema(),DefaultFieldOptions{}),
 MakeStructField("label","label",&Data::label,GetWidgetTransformSchema(),DefaultFieldOptions{}),
 MakeStructField("status","status",&Data::status,GetWidgetTransformSchema(),DefaultFieldOptions{}),
 MakeStructField("summary","summary",&Data::summary,GetWidgetTransformSchema(),DefaultFieldOptions{}),
 MakeField("textColor","textColor",&Data::textColor,ColorFieldOptions{}),
 MakeField("clearColor","clearColor",&Data::clearColor,ColorFieldOptions{}),
 MakeField("failColor","failColor",&Data::failColor,ColorFieldOptions{}),
 MakeField("selectedColor","selectedColor",&Data::selectedColor,ColorFieldOptions{}),
 MakeField("rowSpacing","rowSpacing",&Data::rowSpacing,DragFieldOptions{.dragSpeed=0.1f, .minValue=0, .maxValue=1000000}),
 MakeField("smoothTime","smoothTime",&Data::smoothTime,DragFieldOptions{.dragSpeed=0.1f, .minValue=0, .maxValue=1000000}),
 MakeField("statusWait","statusWait",&Data::statusWait,DragFieldOptions{.dragSpeed=0.1f, .minValue=0, .maxValue=1000000}),
 MakeField("countDuration","countDuration",&Data::countDuration,DragFieldOptions{.dragSpeed=0.1f, .minValue=0, .maxValue=1000000}),
 MakeField("rankWait","rankWait",&Data::rankWait,DragFieldOptions{.dragSpeed=0.1f, .minValue=0, .maxValue=1000000}),
 MakeField("fontSize","fontSize",&Data::fontSize,DragFieldOptions{.dragSpeed=0.1f, .minValue=0, .maxValue=1000000}),
 MakeField("applyPerspective","applyPerspective",&Data::applyPerspective),
 MakeField("applyChromaticEcho","applyChromaticEcho",&Data::applyChromaticEcho)
 }; return schema;
}
}
class ResultUiSettingsAsset : public DataAsset {
    ResultUiSettings::Data m_data;
public:
    ResultUiSettingsAsset() : DataAsset(DataAssetTypeID::getTypeID<ResultUiSettingsAsset>(), "ResultUiSettingsAsset", 0) {}
    const ResultUiSettings::Data& GetData() const { return m_data; }
    std::unique_ptr<DataAsset> CreateDefaultInstance() const override { return std::make_unique<ResultUiSettingsAsset>(); }
    nlohmann::json SerializeData() const override { return FieldSerialization::SerializeFields(m_data, ResultUiSettings::GetSchema()); }
    bool DeserializeDataToApply(const nlohmann::json& json) override {
        auto loaded = m_data;
        if (!FieldSerialization::DeserializeFields(json, loaded, ResultUiSettings::GetSchema())) return false;
        m_data = loaded; return true;
    }
    bool DrawDataOnEditor() override { return FieldEditor::DrawFields(m_data, ResultUiSettings::GetSchema()); }
};

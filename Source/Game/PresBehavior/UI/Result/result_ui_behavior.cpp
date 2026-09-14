#include "result_ui_behavior.h"
#include "Game/Factory/ui_factory.h"
#include "Game/PresBehavior/UI/ui_perspective.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Component/text_component.h"
#include "Engine/Component/slider_component.h"
#include "Engine/Component/rect_transform_component.h"
#include "Engine/Device/direct3d.h"
#include "Engine/Device/mi_fps.h"
#include "Utility/mi_math.h"
#include "External/ImGui/imgui.h"

namespace {
    void Text(UiHandle handle, const std::string& value) { if (auto* text = handle.GetText()) text->SetText(value); }
    float Duration(float value) { return std::isfinite(value) ? (std::max)(0.0f,value) : 0; }
}
void ResultUiBehavior::SetResult(const GameResult& result, const ResultScoring::Score& score) {
     DestroyWidgets();
     m_result = result; m_score = score; m_hasResult = true;
     m_row = 0; m_age = 0; m_phase = Phase::Waves; m_selection = 0;
     // Unreached waves must never be shown, even for a manually supplied preview.
     auto failed = std::find_if(m_result.waves.begin(),m_result.waves.end(),[](const auto& row){return !row.cleared;});
     if (failed != m_result.waves.end()) m_result.waves.erase(failed+1,m_result.waves.end());
}
void ResultUiBehavior::Start() {
    if (m_created || !m_hasResult || !GetOwner() || !GetOwner()->GetScene()) return;

    // テスト実装
    m_result.waves.resize(5);
    m_result.waves[0] = { 1, 50, 50, true };
    m_result.waves[1] = { 2, 50, 50, true };
    m_result.waves[2] = { 3, 50, 50, true };
    m_result.waves[3] = { 4, 50, 50, true };
    m_result.waves[4] = { 5, 50, 50, true };
    m_result.elapsedTime = 120.0f;
    m_result.completed = true;

     auto* scene = GetOwner()->GetScene();
     auto createText = [&] { return UiFactory::CreateUiTextHandle(scene,u8""); };
     for (const auto& wave : m_result.waves) {
          Row row; 
          row.label = createText();
          row.status = createText();

          row.gauge = UiFactory::CreateUiSliderHandle(scene,{0.08f,0.1f,0.14f,1},{0.5f,0.8f,1,1},0);
          
          Text(row.label,"WAVE " + std::to_string(wave.waveNumber) + "  " + std::to_string(wave.points) + " / " + std::to_string(wave.targetPoints));
          row.label.SetActive(false); row.gauge.SetActive(false); row.status.SetActive(false);
          m_rows.push_back(row);
     }
     m_total = createText(); m_complete = createText(); m_time = createText(); m_rank = createText(); m_title = createText(); m_retry = createText();
     Text(m_title,"タイトルへ戻る"); Text(m_retry,"もう一度");
     for (auto handle : {m_total,m_complete,m_time,m_rank,m_title,m_retry}) handle.SetActive(false);
     m_created = true; ApplyLayout();
     if (m_rows.empty()) Advance(Phase::Total);
}
void ResultUiBehavior::ApplyLayout() {
     m_settings = m_asset ? m_asset->GetData() : ResultUiSettings::Data{};
     m_screen = {static_cast<float>(Direct3D_GetBackBufferWidth()),static_cast<float>(Direct3D_GetBackBufferHeight())};
     const auto center = UiLayoutSettings::ResolveGroupPosition(m_settings.placement,m_screen);
     auto perspective = m_settings.perspective;
     perspective.enabled = perspective.enabled && m_settings.applyPerspective;
     const auto presentation = UiPerspective::MakeTransform(perspective,center,center,m_screen);
     auto echoSettings = m_settings.chromaticEcho;
     echoSettings.enabled = echoSettings.enabled && m_settings.applyChromaticEcho;
     const auto echo = UiLayoutSettings::ResolveChromaticEcho(echoSettings,perspective.vanishingPoint,m_settings.placement.screenAnchor,m_screen);
     const float spacing = std::isfinite(m_settings.rowSpacing) ? (std::max)(1.0f,m_settings.rowSpacing) : 55;
     auto layout = [&](UiHandle handle, const UiLayoutSettings::WidgetTransform& transform, float y) {
      handle.SetPosition(center.x+transform.position.x,center.y+transform.position.y+y);
      handle.SetSize(transform.size.x,transform.size.y);
      if (auto* rect = handle.GetRectTransform()) {
       rect->SetPosition({center.x+transform.position.x,center.y+transform.position.y+y,120});
       rect->SetRotation({0,0,DirectX::XMConvertToRadians(transform.rotationDegrees)});
       rect->SetPresentationTransform(presentation); rect->SetChromaticEcho(echo);
      }
      handle.SetColor(m_settings.textColor);
      if (auto* text = handle.GetText()) text->SetFontSize(std::clamp(m_settings.fontSize,1,256));
     };
     for (size_t i=0;i<m_rows.size();++i) {
      auto& row=m_rows[i]; const float y=spacing*static_cast<float>(i);
      layout(row.label,m_settings.label,y); layout(row.gauge,m_settings.gauge,y); layout(row.status,m_settings.status,y);
      row.status.SetColor(m_result.waves[i].cleared ? m_settings.clearColor : m_settings.failColor);
     }
     const float y=spacing*static_cast<float>(m_rows.size()+1);
     layout(m_total,m_settings.summary,y);
     layout(m_complete,m_settings.summary,y+spacing);
     layout(m_time,m_settings.summary,y+spacing*2);
     layout(m_rank,m_settings.summary,y+spacing*(m_result.completed?3:1));
     layout(m_title,m_settings.summary,y+spacing*(m_result.completed?4:2));
     layout(m_retry,m_settings.summary,y+spacing*(m_result.completed?5:3));
     m_revision = m_asset ? m_asset->GetRevision() : 0;
     SetSelection(m_selection);
}

void ResultUiBehavior::SetSelection(int selection) {
     m_selection = std::clamp(selection,0,1);
     m_title.SetColor(m_selection==0?m_settings.selectedColor:m_settings.textColor);
     m_retry.SetColor(m_selection==1?m_settings.selectedColor:m_settings.textColor);
     Text(m_title,m_selection==0?"▶ タイトルへ戻る":"タイトルへ戻る");
     Text(m_retry,m_selection==1?"▶ もう一度":"もう一度");
}

void ResultUiBehavior::Update() {
     if (!m_created) { Start(); if (!m_created) return; }
     if (m_revision!=(m_asset?m_asset->GetRevision():0) || m_screen.x!=Direct3D_GetBackBufferWidth() || m_screen.y!=Direct3D_GetBackBufferHeight()) ApplyLayout();
     const float dt=Duration(FPS_GetUnscaledDeltaTime()); m_age+=dt;
     const float duration=Duration(m_settings.countDuration);
     const float t=duration>0?std::clamp(m_age/duration,0.0f,1.0f):1;
     const double eased=static_cast<double>(t*t*(3-2*t));
     auto count=[&](int from,int to){return t>=1?to:ResultScoring::ClampScore(from+(static_cast<double>(to)-from)*eased);};
     auto total=[&](int value){m_total.SetActive(true);Text(m_total,"累計ポイント  " + std::to_string(value));};
     switch (m_phase) {
     case Phase::Waves: {
      if (m_row>=m_rows.size()) { Advance(Phase::Total); break; }
      auto& row=m_rows[m_row]; const auto& wave=m_result.waves[m_row];
      row.label.SetActive(true);row.gauge.SetActive(true);
      const float target=std::clamp(static_cast<float>(wave.points)/(std::max)(1,wave.targetPoints),0.0f,1.0f);
      row.fill=MiMath::SmoothDamp(row.fill,target,row.velocity,(std::max)(0.0001f,Duration(m_settings.smoothTime)),dt);
      const bool done=std::abs(row.fill-target)<0.0001f || m_age>(std::max)(1.0f,Duration(m_settings.smoothTime)*12);
      if (done) {row.fill=target;row.velocity=0;row.status.SetActive(true);Text(row.status,wave.cleared?"クリア！":"失敗…");Advance(Phase::Status);}
      if(auto* gauge=row.gauge.GetSlider())gauge->SetValue(row.fill);
      break;
     }
     case Phase::Status:
      if(m_age>=Duration(m_settings.statusWait)) {++m_row;Advance(m_row<m_rows.size()?Phase::Waves:Phase::Total);} break;
     case Phase::Total:
      total(count(0,m_score.base));
      if(t>=1)Advance(m_result.completed?Phase::CompleteBonus:Phase::Rank);break;
     case Phase::CompleteBonus:
      m_complete.SetActive(true);Text(m_complete,"コンプリートボーナス  +"+std::to_string(m_score.completeBonus));
      total(count(m_score.base,ResultScoring::ClampScore(static_cast<double>(m_score.base)+m_score.completeBonus)));
      if(t>=1)Advance(Phase::TimeBonus);break;
     case Phase::TimeBonus:
      m_time.SetActive(true);Text(m_time,"タイムボーナス  +"+std::to_string(m_score.timeBonus));
      total(count(ResultScoring::ClampScore(static_cast<double>(m_score.base)+m_score.completeBonus),m_score.total));
      if(t>=1)Advance(Phase::Rank);break;
     case Phase::Rank:
      total(m_score.total);m_rank.SetActive(true);Text(m_rank,"評価  "+std::to_string(m_score.rank)+" / 5");
      if(m_age>=Duration(m_settings.rankWait)){Advance(Phase::Menu);m_title.SetActive(true);m_retry.SetActive(true);}break;
     case Phase::Menu:break;
     }
}
void ResultUiBehavior::DestroyWidgets() {
     for(auto& row:m_rows)for(auto handle:{row.label,row.gauge,row.status})handle.Destroy();
     m_rows.clear();
     for(auto handle:{m_total,m_complete,m_time,m_rank,m_title,m_retry})handle.Destroy();
     m_total={};m_complete={};m_time={};m_rank={};m_title={};m_retry={};m_created=false;
}
void ResultUiBehavior::DrawComponentInspector() {
     ImGui::Text("Result phase: %d | Wave row: %d",static_cast<int>(m_phase),static_cast<int>(m_row));
     if(ImGui::Button("Replay result")){auto result=m_result;auto score=m_score;SetResult(result,score);Start();}
}
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
    const char* RankLetter(int rank) {
        constexpr const char* letters[] = {"D", "D", "C", "B", "A", "S"};
        return letters[std::clamp(rank, 1, 5)];
    }
    DirectX::XMFLOAT3 RankColor(const ResultUiSettings::Data& settings, int rank) {
        switch (std::clamp(rank, 1, 5)) {
        case 5: return settings.rankSColor;
        case 4: return settings.rankAColor;
        case 3: return settings.rankBColor;
        case 2: return settings.rankCColor;
        default: return settings.rankDColor;
        }
    }
    const std::string& RankRewardMaterial(const ResultUiSettings::Data& settings, int rank) {
        switch (std::clamp(rank, 1, 5)) {
        case 5: return settings.rankSRewardText;
        case 4: return settings.rankARewardText;
        case 3: return settings.rankBRewardText;
        case 2: return settings.rankCRewardText;
        default: return settings.rankDRewardText;
        }
    }
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

     auto* scene = GetOwner()->GetScene();
     auto createText = [&] { return UiFactory::CreateUiTextHandle(scene,u8""); };
     for (const auto& wave : m_result.waves) {
          Row row; 
          row.label = createText();
          row.status = createText();

          const auto& settings = m_asset ? m_asset->GetData() : m_settings;
          row.gauge = UiFactory::CreateUiSliderHandle(scene,settings.gaugeBgColor,settings.gaugeFillColor,0);
          
          Text(row.label,"WAVE " + std::to_string(wave.waveNumber) + "  " + std::to_string(wave.points) + " / " + std::to_string(wave.targetPoints));
          row.label.SetActive(false); row.gauge.SetActive(false); row.status.SetActive(false);
          m_rows.push_back(row);
     }
     m_total = createText(); m_complete = createText(); m_time = createText(); m_rank = createText();
     m_rankRewardMaterial = createText(); m_rankRewardSuffix = createText();
     m_title = createText(); m_retry = createText();
     m_selectionBackground = UiFactory::CreateUiImageHandle(scene, L"asset/Texture/white.bmp");
     Text(m_title,"タイトルへ戻る"); Text(m_retry,"もう一度");
     for (auto handle : {m_total,m_complete,m_time,m_rank,m_rankRewardMaterial,m_rankRewardSuffix,m_title,m_retry,m_selectionBackground}) handle.SetActive(false);
     m_created = true; ApplyLayout();
     if (m_rows.empty()) Advance(Phase::Total);
}
void ResultUiBehavior::ApplyLayout() {
    m_settings = m_asset ? m_asset->GetData() : ResultUiSettings::Data{};
    m_screen = {static_cast<float>(Direct3D_GetBackBufferWidth()), static_cast<float>(Direct3D_GetBackBufferHeight())};
    const float spacing = std::isfinite(m_settings.rowSpacing) ? (std::max)(1.0f, m_settings.rowSpacing) : 55;
    auto layout = [&](UiHandle handle, const UiLayoutSettings::WidgetTransform& transform,
                      const UiLayoutSettings::GroupPlacement& placement, float y = 0, float layer = 120) {
        const auto center = UiLayoutSettings::ResolveGroupPosition(placement, m_screen);
        auto perspective = m_settings.perspective;
        perspective.enabled = perspective.enabled && m_settings.applyPerspective;
        const auto presentation = UiPerspective::MakeTransform(perspective, center, center, m_screen);
        auto echoSettings = m_settings.chromaticEcho;
        echoSettings.enabled = echoSettings.enabled && m_settings.applyChromaticEcho;
        const auto echo = UiLayoutSettings::ResolveChromaticEcho(echoSettings, perspective.vanishingPoint, placement.screenAnchor, m_screen);
        handle.SetSize(transform.size.x, transform.size.y);
        if (auto* rect = handle.GetRectTransform()) {
            rect->SetPosition({center.x + transform.position.x, center.y + transform.position.y + y, layer});
            rect->SetRotation({0, 0, DirectX::XMConvertToRadians(transform.rotationDegrees)});
            rect->SetPresentationTransform(presentation);
            rect->SetChromaticEcho(echo);
        }
        handle.SetColor(m_settings.textColor);
        if (auto* text = handle.GetText()) text->SetFontSize(std::clamp(m_settings.fontSize, 1, 256));
    };
    for (size_t i = 0; i < m_rows.size(); ++i) {
        auto& row = m_rows[i];
        const float y = spacing * static_cast<float>(i);
        layout(row.label, m_settings.label, m_settings.wavePlacement, y);
        layout(row.gauge, m_settings.gauge, m_settings.wavePlacement, y);
        if (auto* gauge = row.gauge.GetSlider()) {
            gauge->SetBgColor(m_settings.gaugeBgColor);
            gauge->SetFillColor(m_settings.gaugeFillColor);
        }
        layout(row.status, m_settings.status, m_settings.wavePlacement, y);
        row.status.SetColor(m_result.waves[i].cleared ? m_settings.clearColor : m_settings.failColor);
    }
    layout(m_total, m_settings.summary, m_settings.scorePlacement);
    layout(m_complete, m_settings.summary, m_settings.scorePlacement, spacing);
    layout(m_time, m_settings.summary, m_settings.scorePlacement, spacing * 2);
    layout(m_rank, m_settings.rank, m_settings.rankPlacement);
    layout(m_rankRewardMaterial, m_settings.rankRewardMaterial, m_settings.rankPlacement);
    layout(m_rankRewardSuffix, m_settings.rankRewardSuffix, m_settings.rankPlacement);
    const auto rankColor = RankColor(m_settings, m_score.rank);
    m_rank.SetColor(rankColor);
    m_rankRewardMaterial.SetColor(rankColor);
    layout(m_title, m_settings.title, m_settings.menuPlacement);
    layout(m_retry, m_settings.retry, m_settings.menuPlacement);
    layout(m_selectionBackground, m_settings.selection, m_settings.menuPlacement, 0, 119);
    m_selectionBackground.SetColor(m_settings.selectionColor);
    m_selectionBackground.SetAlpha(std::isfinite(m_settings.selectionOpacity) ? std::clamp(m_settings.selectionOpacity, 0.0f, 1.0f) : 0.8f);
    m_revision = m_asset ? m_asset->GetRevision() : 0;
    const auto target = m_selection == 0 ? m_settings.title.position : m_settings.retry.position;
    m_selectionMotion.MoveTo({target.x + m_settings.selection.position.x, target.y + m_settings.selection.position.y}, 0);
    ApplySelectionPosition();
    SetSelection(m_selection);
    // Layout has restored the unshaken positions, including after a resize.
    for (auto& shake : m_shakes) {
        if (auto* rect = shake.handle.GetRectTransform()) {
            const auto position = rect->GetPosition();
            shake.origin = {position.x, position.y};
        }
    }
}

void ResultUiBehavior::ShowWithShake(UiHandle handle) {
    auto* object = handle.GetGameObject();
    if (!object || object->GetActive()) return;
    handle.SetActive(true);
    if (auto* rect = handle.GetRectTransform()) {
        const auto position = rect->GetPosition();
        m_shakes.push_back({handle, {position.x, position.y}, 0});
    }
}

void ResultUiBehavior::UpdateShakes(float dt) {
    const float duration = Duration(m_settings.shakeDuration);
    const float amplitude = Duration(m_settings.shakeAmplitude);
    const float frequency = Duration(m_settings.shakeFrequency);
    for (auto& shake : m_shakes) {
        shake.age += dt;
        const float envelope = duration > 0 ? std::clamp(1 - shake.age / duration, 0.0f, 1.0f) : 0;
        const float phase = shake.age * frequency * DirectX::XM_2PI;
        const float x = std::cos(phase) * amplitude * envelope;
        const float y = std::sin(phase * 1.3f) * amplitude * envelope * 0.5f;
        shake.handle.SetPosition(shake.origin.x + x, shake.origin.y + y);
    }
    std::erase_if(m_shakes, [&](const Shake& shake) { return !shake.handle.IsValid() || shake.age >= duration; });
}

void ResultUiBehavior::ApplySelectionPosition() {
    const auto center = UiLayoutSettings::ResolveGroupPosition(m_settings.menuPlacement, m_screen);
    const auto offset = m_selectionMotion.GetPosition();
    m_selectionBackground.SetPosition(center.x + offset.x, center.y + offset.y);
}

void ResultUiBehavior::SetSelection(int selection) {
    const int next = std::clamp(selection, 0, 1);
    if (next != m_selection) {
        const auto target = next == 0 ? m_settings.title.position : m_settings.retry.position;
        m_selectionMotion.MoveTo({target.x + m_settings.selection.position.x, target.y + m_settings.selection.position.y}, m_settings.selectionMoveDuration);
    }
    m_selection = next;
    m_title.SetColor(m_selection == 0 ? m_settings.selectedColor : m_settings.textColor);
    m_retry.SetColor(m_selection == 1 ? m_settings.selectedColor : m_settings.textColor);
    Text(m_title, "タイトルへ戻る");
    Text(m_retry, "もう一度");
}
void ResultUiBehavior::Update() {
     if (!m_created) { Start(); if (!m_created) return; }
     if (m_revision!=(m_asset?m_asset->GetRevision():0) || m_screen.x!=Direct3D_GetBackBufferWidth() || m_screen.y!=Direct3D_GetBackBufferHeight()) ApplyLayout();
     const float dt=Duration(FPS_GetUnscaledDeltaTime()); m_age+=dt;
     m_selectionMotion.Update(dt);
     ApplySelectionPosition();
     const float duration=Duration(m_settings.countDuration);
     const float t=duration>0?std::clamp(m_age/duration,0.0f,1.0f):1;
     const double eased=static_cast<double>(t*t*(3-2*t));
     auto count=[&](int from,int to){return t>=1?to:ResultScoring::ClampScore(from+(static_cast<double>(to)-from)*eased);};
     auto total=[&](int value){ShowWithShake(m_total);Text(m_total,"累計ポイント  " + std::to_string(value));};
     switch (m_phase) {
     case Phase::Waves: {
      if (m_row>=m_rows.size()) { Advance(Phase::Total); break; }
      auto& row=m_rows[m_row]; const auto& wave=m_result.waves[m_row];
      row.label.SetActive(true);row.gauge.SetActive(true);
      const float target=std::clamp(static_cast<float>(wave.points)/(std::max)(1,wave.targetPoints),0.0f,1.0f);
      row.fill=MiMath::SmoothDamp(row.fill,target,row.velocity,(std::max)(0.0001f,Duration(m_settings.smoothTime)),dt);
      const bool done=std::abs(row.fill-target)<0.0001f || m_age>(std::max)(1.0f,Duration(m_settings.smoothTime)*12);
      if (done) {row.fill=target;row.velocity=0;ShowWithShake(row.status);Text(row.status,wave.cleared?"クリア！":"失敗…");Advance(Phase::Status);}
      if(auto* gauge=row.gauge.GetSlider())gauge->SetValue(row.fill);
      break;
     }
     case Phase::Status:
      if(m_age>=Duration(m_settings.statusWait)) {++m_row;Advance(m_row<m_rows.size()?Phase::Waves:Phase::Total);} break;
     case Phase::Total:
      total(count(0,m_score.base));
      if(t>=1)Advance(m_result.completed?Phase::CompleteBonus:Phase::Rank);break;
     case Phase::CompleteBonus:
      ShowWithShake(m_complete);Text(m_complete,"コンプリートボーナス  +"+std::to_string(m_score.completeBonus));
      total(count(m_score.base,ResultScoring::ClampScore(static_cast<double>(m_score.base)+m_score.completeBonus)));
      if(t>=1)Advance(Phase::TimeBonus);break;
     case Phase::TimeBonus:
      ShowWithShake(m_time);Text(m_time,"タイムボーナス  +"+std::to_string(m_score.timeBonus));
      total(count(ResultScoring::ClampScore(static_cast<double>(m_score.base)+m_score.completeBonus),m_score.total));
      if(t>=1)Advance(Phase::Rank);break;
     case Phase::Rank:
      total(m_score.total);ShowWithShake(m_rank);Text(m_rank,RankLetter(m_score.rank));
      m_rank.SetColor(RankColor(m_settings,m_score.rank));
      if(m_age>=Duration(m_settings.rankWait)) Advance(Phase::RankReward);break;
     case Phase::RankReward:
      ShowWithShake(m_rankRewardMaterial);ShowWithShake(m_rankRewardSuffix);
      Text(m_rankRewardMaterial,RankRewardMaterial(m_settings,m_score.rank));
      Text(m_rankRewardSuffix,m_settings.rankRewardSuffixText);
      m_rankRewardMaterial.SetColor(RankColor(m_settings,m_score.rank));
      m_rankRewardSuffix.SetColor(m_settings.textColor);
      if(m_age>=Duration(m_settings.rankRewardWait)){Advance(Phase::Menu);m_title.SetActive(true);m_retry.SetActive(true);m_selectionBackground.SetActive(true);}break;
     case Phase::Menu:break;
     }
     UpdateShakes(dt);
}
void ResultUiBehavior::DestroyWidgets() {
     m_shakes.clear();
     for(auto& row:m_rows)for(auto handle:{row.label,row.gauge,row.status})handle.Destroy();
     m_rows.clear();
     for(auto handle:{m_total,m_complete,m_time,m_rank,m_rankRewardMaterial,m_rankRewardSuffix,m_title,m_retry,m_selectionBackground})handle.Destroy();
     m_total={};m_complete={};m_time={};m_rank={};m_rankRewardMaterial={};m_rankRewardSuffix={};
     m_title={};m_retry={};m_selectionBackground={};m_selectionMotion={};m_created=false;
}
void ResultUiBehavior::DrawComponentInspector() {
     ImGui::Text("Result phase: %d | Wave row: %d",static_cast<int>(m_phase),static_cast<int>(m_row));
     if(ImGui::Button("Replay result")){auto result=m_result;auto score=m_score;SetResult(result,score);Start();}
}

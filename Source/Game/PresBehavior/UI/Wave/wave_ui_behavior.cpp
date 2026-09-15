#include "wave_ui_behavior.h"
#include "Game/ControllerBehavior/Wave/wave_controller_behavior.h"
#include "Game/ControllerBehavior/game_controller_locator.h"
#include "Engine/Core/game_object.h"
#include "Engine/Core/scene_interface.h"
#include "Engine/Component/transform_component.h"
#include "Engine/Device/mi_fps.h"
#include "External/ImGui/imgui.h"

void WaveUiBehavior::Start()
{
    if (m_created || !GetOwner() || !GetOwner()->GetScene()) return;
    auto* scene = GetOwner()->GetScene();
    m_number.Initialize(scene, "WaveUi.Number");
    m_points.Initialize(scene, "WaveUi.Points", true);
    m_phase.Initialize(scene, "WaveUi.Phase");
    m_timer.Initialize(scene, "WaveUi.Timer");
    m_created = true;
    ApplyLayout();
}

void WaveUiBehavior::ApplyLayout()
{
    m_settings = m_settingsAsset ? m_settingsAsset->GetData() : WaveUiSettings::Data{};
    WaveUiSettings::Sanitize(m_settings);
    m_screen = {static_cast<float>(Direct3D_GetBackBufferWidth()), static_cast<float>(Direct3D_GetBackBufferHeight())};
    m_number.ApplyLayout(m_settings.number, m_screen);
    m_points.ApplyLayout(m_settings.points.text, m_screen);
    m_points.ApplyGauge(m_settings.points);
    m_phase.ApplyLayout(m_settings.phase, m_screen);
    m_timer.ApplyLayout(m_settings.timer, m_screen);
    m_number.ApplyPresentation(m_settings, m_screen);
    m_points.ApplyPresentation(m_settings, m_screen);
    m_phase.ApplyPresentation(m_settings, m_screen);
    m_timer.ApplyPresentation(m_settings, m_screen);
    m_revision = m_settingsAsset ? m_settingsAsset->GetRevision() : 0;
    m_dirty = false;
}

void WaveUiBehavior::Update()
{
    if (!m_created) return;
    if (m_dirty || m_revision != (m_settingsAsset ? m_settingsAsset->GetRevision() : 0)
        || m_screen.x != Direct3D_GetBackBufferWidth() || m_screen.y != Direct3D_GetBackBufferHeight()) ApplyLayout();
    const float rawDt = FPS_GetUnscaledDeltaTime();
    const float dt = std::isfinite(rawDt) ? (std::max)(0.0f, rawDt) : 0;
    auto* controller = Game::Wave();
    bool phaseChanged = false;
    if (controller != m_controller) {
        m_controller = controller;
        m_eventCursor = controller ? controller->GetDefeatSerial() : 0;
        m_lastWave = -1; m_lastPoints = -1;
        m_phaseMotion.Reset();
        for (auto& popup : m_popups) { popup.active = false; popup.handle.SetActive(false); }
    }
    if (controller) {
        const auto& progress = controller->GetProgress();
        phaseChanged = m_lastWave < 0 || m_lastState != progress.state || m_lastWave != progress.waveNumber;
        if (phaseChanged) m_phaseMotion.Begin(progress.state);
        if (m_lastWave >= 0 && m_lastWave != progress.waveNumber) m_number.Pulse(m_settings.pulseDuration);
        if (m_lastPoints >= 0 && m_lastPoints != progress.wavePoints) m_points.Pulse(m_settings.pulseDuration);
        if (m_lastWave >= 0 && m_lastState != progress.state) m_phase.Pulse(m_settings.pulseDuration);
        m_number.SetText("WAVE " + std::to_string(progress.waveNumber) + " / " + std::to_string(controller->GetWaveCount()));
        m_points.SetText(std::to_string(progress.wavePoints) + " / " + std::to_string(progress.targetPoints) + "  合計 " + std::to_string(progress.totalScore));
        m_points.SetFill(progress.targetPoints > 0 ? static_cast<float>(progress.wavePoints) / progress.targetPoints : 0);
        
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << progress.elapsedTime;
        m_timer.SetText(oss.str());
        
        const int seconds = static_cast<int>(std::ceil(progress.remainingTime));
        std::string phase;
        switch (progress.state) {
        case WaveProgress::State::WaitingForWorld: phase = "WAIT"; break;
        case WaveProgress::State::Preparing: phase = "READY？ " + std::to_string(seconds); break;
        case WaveProgress::State::Battle: {
            if (progress.waveNumber == 1) {
                phase = "鉄屑を撃ち倒せ！";
            }
            else if (progress.waveNumber == 2) {
                phase = "バラバラにしてやれ！";
            }
            else if (progress.waveNumber == 3) {
                phase = "撃って、砕いて、突き進め！";
            }
            else if (progress.waveNumber == 4) {
                phase = "残らずスクラップにしろ！";
            }
            else {
                phase = "兎の火力を思い知れ！";
            }
            break;
        }
        case WaveProgress::State::ClearImpact:
            phase = "討伐成功！";
            break;
        case WaveProgress::State::Intermission:
            phase = progress.waveNumber >= controller->GetWaveCount() ? "WAVE クリア！"
                : "WAVE クリア！ / 次の WAVE まで  " + std::to_string(seconds);
            break;
        case WaveProgress::State::Complete: phase = "全 WAVES クリア！！"; break;
        case WaveProgress::State::GameOver: phase = "GAME OVER"; break;
        }
        m_phase.SetText(phase);
        m_lastState = progress.state; m_lastWave = progress.waveNumber; m_lastPoints = progress.wavePoints;
        for (const auto& event : controller->GetDefeatEvents()) {
            if (event.serial <= m_eventCursor) continue;
            if (m_settings.popupEnabled) SpawnPopup(event.points, event.position);
            m_eventCursor = event.serial;
        }
    }
    else {
        m_number.SetText("WAVE --"); m_points.SetText("-- / --"); m_points.SetFill(0); m_phase.SetText("WAITING FOR WAVE CONTROLLER");
    }
    if (!phaseChanged) m_phaseMotion.Update(dt);
    ApplyPhaseMotion();
    m_number.Update(dt, m_settings); m_points.Update(dt, m_settings); m_phase.Update(dt, m_settings);
    m_phase.ApplyPresentation(m_settings, m_screen);
    UpdatePopups(dt);
}

void WaveUiBehavior::ApplyPhaseMotion()
{
    const float t = m_phaseMotion.GetBlend(m_settings.phaseMoveDelay, m_settings.phaseMoveDuration);
    auto lerp = [t](DirectX::XMFLOAT2 a, DirectX::XMFLOAT2 b) {
        return DirectX::XMFLOAT2{a.x + (b.x-a.x)*t, a.y + (b.y-a.y)*t};
    };
    const auto& a = m_settings.phase.placement;
    const auto& b = m_settings.phasePlacementB;
    const UiLayoutSettings::GroupPlacement placement = {lerp(a.screenAnchor,b.screenAnchor),lerp(a.position,b.position)};
    m_phase.ApplyAnimatedLayout(placement, lerp(m_settings.phase.label.size,m_settings.phaseScaleB),m_screen);
}

void WaveUiBehavior::SpawnPopup(int points, DirectX::XMFLOAT3 position)
{
    auto& popup = m_popups[m_nextPopup++ % m_popups.size()];
    if (!popup.handle.IsValid()) {
        auto* object = GetOwner()->GetScene()->CreateGameObject();
        object->SetName("WaveUi.DefeatPopup");
        object->AddComponent<TransformComponent>();
        auto* text = object->AddComponent<TextComponent>();
        text->SetFontPath("asset/Font/Makinas-4-Square.otf"); text->SetFontSize(32); text->SetCenter(true);
        popup.handle = UiHandle(object);
    }
    position.y += m_settings.popupHeightOffset;
    popup.origin = position; popup.age = 0; popup.duration = m_settings.popupDuration; popup.rise = m_settings.popupRiseDistance; popup.active = true;
    popup.handle.SetActive(true);
    popup.handle.GetTransform()->SetPosition(position);
    popup.handle.GetTransform()->SetScaling({m_settings.popupScale, m_settings.popupScale, m_settings.popupScale});
    popup.handle.GetText()->SetText("+" + std::to_string(points));
    popup.handle.SetColor(m_settings.popupColor); popup.handle.SetAlpha(1);
}

void WaveUiBehavior::UpdatePopups(float dt)
{
    for (auto& popup : m_popups) {
        if (!popup.active) continue;
        popup.age += dt;
        const float t = std::clamp(popup.age / popup.duration, 0.0f, 1.0f);
        auto position = popup.origin; position.y += popup.rise * t;
        if (auto* transform = popup.handle.GetTransform()) transform->SetPosition(position);
        popup.handle.SetAlpha(1 - t);
        if (t >= 1) { popup.active = false; popup.handle.SetActive(false); }
    }
}

void WaveUiBehavior::DestroyWidgets()
{
    m_phaseMotion.Reset();
    m_number.Destroy(); m_points.Destroy(); m_phase.Destroy();
    for (auto& popup : m_popups) { popup.handle.Destroy(); popup = {}; }
    m_created = false; m_controller = nullptr; m_lastWave = -1; m_lastPoints = -1;
}

void WaveUiBehavior::DrawComponentInspector()
{
    ImGui::TextUnformatted("Layout: asset/Data/wave_ui_settings.data.json (DataAsset editor)");
    if (ImGui::Button("Preview Phase Pulse")) m_phase.Pulse(m_settings.pulseDuration);
    ImGui::Text("Widgets created: %s", m_created ? "yes" : "no");
}

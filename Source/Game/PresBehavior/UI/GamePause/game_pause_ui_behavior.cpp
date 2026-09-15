#include "game_pause_ui_behavior.h"
#include "Game/Factory/ui_factory.h"
#include "Game/PresBehavior/UI/ui_perspective.h"
#include "Engine/Core/game_object.h"
#include "Engine/Component/rect_transform_component.h"
#include "Engine/Component/text_component.h"
#include "Engine/Component/image_component.h"
#include "Engine/Device/direct3d.h"
#include "Engine/Device/mi_fps.h"
#include "Engine/Graphics/texture_repository.h"
#include "Engine/engine_service_locator.h"
#include "External/ImGui/imgui.h"
#include <algorithm>
#include <cmath>

namespace {
    void ApplyImage(UiHandle handle, const std::string& path, DirectX::XMFLOAT3 color, float alpha) {
        if (auto* image = handle.GetImage(); image && TEXTURE_REPOSITORY) {
            auto* texture = TEXTURE_REPOSITORY->GetTextureResource(path);
            if (!texture) texture = TEXTURE_REPOSITORY->GetTextureResource("asset/Texture/white.bmp");
            if (texture) image->SetTextureResource(texture);
        }
        handle.SetColor(color); handle.SetAlpha(alpha);
    }
}

const GamePauseUiSettings::Data& GamePauseUiBehavior::Settings() const {
    static const GamePauseUiSettings::Data defaults;
    return m_settings ? m_settings->GetData() : defaults;
}

void GamePauseUiBehavior::Start() {
    if (m_created || !GetOwner() || !GetOwner()->GetScene()) return;
    auto* scene = GetOwner()->GetScene();
    m_dimmer = UiFactory::CreateUiImageHandle(scene, L"asset/Texture/white.bmp");
    m_elements[0] = UiFactory::CreateUiImageHandle(scene, L"asset/Texture/white.bmp");
    m_elements[1] = UiFactory::CreateUiImageHandle(scene, L"asset/Texture/white.bmp");
    for (size_t i = 2; i < m_elements.size(); ++i) m_elements[i] = UiFactory::CreateUiTextHandle(scene, u8"");
    const char* names[] = {"GamePauseUi.Panel", "GamePauseUi.Selection", "GamePauseUi.Message", "GamePauseUi.Yes", "GamePauseUi.No"};
    for (size_t i = 0; i < m_elements.size(); ++i) {
        if (auto* object = m_elements[i].GetGameObject()) object->SetName(names[i]);
        if (auto* rect = m_elements[i].GetRectTransform()) rect->SetPosition({0, 0, 201.0f + static_cast<float>(i)});
    }
    if (auto* object = m_dimmer.GetGameObject()) object->SetName("GamePauseUi.Dimmer");
    if (auto* rect = m_dimmer.GetRectTransform()) rect->SetPosition({0, 0, 200});
    m_created = true;
    ApplySettings();
}

DirectX::XMFLOAT2 GamePauseUiBehavior::SelectionTarget() const {
    const auto& s = Settings();
    const auto item = m_yesSelected ? s.yes.position : s.no.position;
    return {item.x + s.selection.position.x, item.y + s.selection.position.y};
}

void GamePauseUiBehavior::ApplySettings() {
    if (!m_created) return;
    const auto& s = Settings();
    if (auto* text = m_elements[2].GetText()) { text->SetText(s.messageText); text->SetFontSize(s.fontSize); }
    if (auto* text = m_elements[3].GetText()) { text->SetText(s.yesText); text->SetFontSize(s.fontSize); }
    if (auto* text = m_elements[4].GetText()) { text->SetText(s.noText); text->SetFontSize(s.fontSize); }
    ApplyImage(m_elements[0], s.panelImagePath, s.panelColor, 1);
    ApplyImage(m_elements[1], s.selectionImagePath, s.selectionColor, s.selectionOpacity);
    m_dimmer.SetColor({0, 0, 0}); m_dimmer.SetAlpha(s.dimmerOpacity);
    m_selectionCurrent = m_selectionTarget = SelectionTarget();
    m_revision = m_settings ? m_settings->GetRevision() : 0;
    ApplyView();
}

void GamePauseUiBehavior::ApplyView() {
    if (!m_created) return;
    const auto& s = Settings();
    const auto center = UiLayoutSettings::ResolveGroupPosition(s.placement, m_screenSize);
    auto perspective = s.perspective; perspective.enabled = perspective.enabled && s.applyPerspective;
    const auto transform = UiPerspective::MakeTransform(perspective, center, center, m_screenSize);
    auto echoSettings = s.chromaticEcho; echoSettings.enabled = echoSettings.enabled && s.applyChromaticEcho;
    const auto echo = UiLayoutSettings::ResolveChromaticEcho(echoSettings, perspective.vanishingPoint, s.placement.screenAnchor, m_screenSize);
    const UiLayoutSettings::WidgetTransform layouts[] = {s.panel, s.selection, s.message, s.yes, s.no};
    for (size_t i = 0; i < m_elements.size(); ++i) {
        const auto position = i == 1 ? m_selectionCurrent : layouts[i].position;
        m_elements[i].SetActive(m_visible);
        m_elements[i].SetPosition(center.x + position.x, center.y + position.y);
        m_elements[i].SetSize(layouts[i].size.x, layouts[i].size.y);
        if (auto* rect = m_elements[i].GetRectTransform()) {
            rect->SetRotation({0, 0, DirectX::XMConvertToRadians(layouts[i].rotationDegrees)});
            rect->SetPresentationTransform(transform); rect->SetChromaticEcho(echo);
        }
    }
    m_elements[2].SetColor(s.textColor);
    m_elements[3].SetColor(m_yesSelected ? s.selectedTextColor : s.textColor);
    m_elements[4].SetColor(m_yesSelected ? s.textColor : s.selectedTextColor);
    m_dimmer.SetActive(m_visible);
    m_dimmer.SetPosition(m_screenSize.x * .5f, m_screenSize.y * .5f);
    m_dimmer.SetSize(m_screenSize.x, m_screenSize.y);
}

void GamePauseUiBehavior::Update() {
    if (!m_created) return;
    const DirectX::XMFLOAT2 size = {static_cast<float>(Direct3D_GetBackBufferWidth()), static_cast<float>(Direct3D_GetBackBufferHeight())};
    const auto revision = m_settings ? m_settings->GetRevision() : 0;
    if (revision != m_revision || size.x != m_screenSize.x || size.y != m_screenSize.y) { m_screenSize = size; ApplySettings(); }
    if (m_selectionDuration > 0) {
        m_selectionElapsed = (std::min)(m_selectionElapsed + FPS_GetUnscaledDeltaTime(), m_selectionDuration);
        float t = m_selectionElapsed / m_selectionDuration; t = t * t * (3 - 2 * t);
        m_selectionCurrent = {m_selectionStart.x + (m_selectionTarget.x - m_selectionStart.x) * t,
            m_selectionStart.y + (m_selectionTarget.y - m_selectionStart.y) * t};
        if (m_selectionElapsed >= m_selectionDuration) m_selectionDuration = 0;
    }
    ApplyView();
}

void GamePauseUiBehavior::SetVisible(bool visible) { m_visible = visible; ApplyView(); }
void GamePauseUiBehavior::SetSelection(bool yesSelected, bool animate) {
    m_yesSelected = yesSelected;
    m_selectionStart = m_selectionCurrent; m_selectionTarget = SelectionTarget(); m_selectionElapsed = 0;
    m_selectionDuration = animate ? (std::max)(Settings().selectionMoveDuration, 0.0f) : 0;
    if (m_selectionDuration == 0) m_selectionCurrent = m_selectionTarget;
    ApplyView();
}
void GamePauseUiBehavior::OnDestroy() {
    m_dimmer.Destroy();
    for (auto& element : m_elements) element.Destroy();
    m_created = false;
}
void GamePauseUiBehavior::DrawComponentInspector() {
    bool visible = m_visible;
    if (ImGui::Checkbox("Visible", &visible)) SetVisible(visible);
    bool yes = m_yesSelected;
    if (ImGui::Checkbox("Select yes", &yes)) SetSelection(yes);
}

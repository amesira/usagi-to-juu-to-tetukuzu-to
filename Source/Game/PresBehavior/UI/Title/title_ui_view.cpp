#include "title_ui_view.h"
#include "Game/PresBehavior/UI/ui_perspective.h"
#include "Engine/Component/rect_transform_component.h"

void TitleUiView::ApplyGroup(TitleUi::Group& group, const TitleUiSettings::Data& settings,
    DirectX::XMFLOAT2 screenSize, int selectionIndex, DirectX::XMFLOAT2 selectionPosition)
{
    const auto center = UiLayoutSettings::ResolveGroupPosition(group.settings.placement, screenSize);
    auto perspective = settings.perspective;
    perspective.enabled = perspective.enabled && group.settings.applyPerspective;
    const auto transform = UiPerspective::MakeTransform(perspective, center, center, screenSize);
    auto echoSettings = settings.chromaticEcho;
    echoSettings.enabled = echoSettings.enabled && group.settings.applyChromaticEcho;
    const auto echo = UiLayoutSettings::ResolveChromaticEcho(echoSettings,
        perspective.vanishingPoint, group.settings.placement.screenAnchor, screenSize);
    for (size_t i = 0; i < group.elements.size(); ++i) {
        auto& element = group.elements[i];
        const auto position = static_cast<int>(i) == selectionIndex ? selectionPosition : element.layout.position;
        element.handle.SetActive(group.visible);
        element.handle.SetPosition(center.x + position.x, center.y + position.y);
        element.handle.SetSize(element.layout.size.x, element.layout.size.y);
        if (auto* rect = element.handle.GetRectTransform()) {
            rect->SetRotation({0, 0, DirectX::XMConvertToRadians(element.layout.rotationDegrees)});
            rect->SetPresentationTransform(transform);
            rect->SetChromaticEcho(echo);
        }
    }
}
void TitleUiView::ApplyDimmer(TitleUiExitPopup& popup, DirectX::XMFLOAT2 size) {
    popup.dimmer.SetActive(popup.group.visible);
    popup.dimmer.SetPosition(size.x * 0.5f, size.y * 0.5f);
    popup.dimmer.SetSize(size.x, size.y);
}

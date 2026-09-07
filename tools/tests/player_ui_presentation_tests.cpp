#include "Game/PresBehavior/UI/Player/player_ui_presentation.h"
#include "Game/PresBehavior/UI/Player/player_ui_view.h"
#include <cassert>
#include <cmath>
#include <vector>
#include <iostream>

// Replace only handle-to-engine dispatch; exercise real Presentation and View code.
static std::vector<DirectX::XMFLOAT2> positions;
void UiHandle::SetPosition(float x, float y) { positions.push_back({x,y}); }
void UiHandle::SetAlpha(float) {}
static bool Near(float a, float b) { return std::abs(a-b) < 0.0001f; }
int main()
{
    PlayerUiContext context;
    PlayerUiView view;
    PlayerUiPresentation presentation;
    context.view = &view;
    context.presentation = &presentation;
    presentation.Initialize();
    const auto hp = PlayerUi::WidgetGroupID::HealthBar;
    const auto ammo = PlayerUi::WidgetGroupID::AmmoCount;
    auto& group = context.widgetGroups[static_cast<size_t>(hp)];
    group.widgets.resize(3);
    group.offsetPositions = {{-10,0},{10,0}};
    view.SetWidgetGroupPosition(context,hp,{100,200});
    assert(positions.size()==2 && Near(positions[0].x,90) && Near(positions[1].x,110));
    presentation.FadeWidgetGroup(context,hp,0,1);
    presentation.ShakeWidgetGroup(context,hp,10,1);
    presentation.Update(context,0.125f);
    assert(Near(group.currentAlpha,0.875f));
    assert(std::abs(group.shakeOffset.y)>0.01f);
    assert(Near(group.currentCenterPosition.x,100+group.shakeOffset.x));
    const auto offset = group.shakeOffset;
    view.SetWidgetGroupPosition(context,hp,{300,400});
    assert(Near(group.currentCenterPosition.y,400+offset.y));
    presentation.FadeWidgetGroup(context,hp,1,0.5f);
    presentation.Update(context,0.25f);
    assert(Near(group.currentAlpha,0.9375f));
    presentation.CancelFadeWidgetGroup(context,hp);
    presentation.Update(context,0.1f);
    assert(Near(group.currentAlpha,0.9375f));
    presentation.CancelShakeWidgetGroup(context,hp);
    assert(Near(group.currentCenterPosition.x,300) && Near(group.currentCenterPosition.y,400));
    presentation.ShakeWidgetGroup(context,hp,5,1);
    presentation.Update(context,0.02f);
    presentation.ShakeWidgetGroup(context,hp,2,0.1f);
    assert(Near(group.shakeOffset.x,0));
    presentation.Update(context,0.2f);
    assert(Near(group.currentCenterPosition.x,300) && Near(group.shakeOffset.y,0));
    presentation.FadeWidgetGroup(context,hp,-1,0);
    assert(Near(group.currentAlpha,0));
    presentation.FadeWidgetGroup(context,hp,1,0.1f);
    presentation.Update(context,1);
    assert(Near(group.currentAlpha,1));
    presentation.FadeWidgetGroup(context,ammo,0,1);
    presentation.Update(context,0.5f);
    assert(Near(group.currentAlpha,1));
    assert(Near(context.widgetGroups[static_cast<size_t>(ammo)].currentAlpha,0.5f));
    presentation.Update(context,-1);
    assert(Near(context.widgetGroups[static_cast<size_t>(ammo)].currentAlpha,0.5f));
    presentation.CancelAll(context);
    presentation.Update(context,1);
    assert(Near(context.widgetGroups[static_cast<size_t>(ammo)].currentAlpha,0.5f));
    presentation.FadeWidgetGroup(context,static_cast<PlayerUi::WidgetGroupID>(999),0,1);
    presentation.ShakeWidgetGroup(context,PlayerUi::WidgetGroupID::None,1,1);
    context.view = nullptr;
    presentation.Update(context,1);

    PlayerUi::ShakeTask a,b;
    a.m_intensity=b.m_intensity=10;
    a.m_duration=b.m_duration=1;
    a.Start(); b.Start();
    a.Update(0.2f);
    b.Update(0.1f); b.Update(0.1f);
    assert(Near(a.m_currentOffset.x,b.m_currentOffset.x));
    assert(Near(a.m_currentOffset.y,b.m_currentOffset.y));
    std::cout << "Fade retarget/cancel, shake restore/restart, moving anchors, parallel groups, invalid input and frame timing passed.\n";
}
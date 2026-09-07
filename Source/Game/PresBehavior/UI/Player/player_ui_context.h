// player_ui_context.h
#pragma once
#include <vector>
#include <DirectXMath.h>
#include "Game/PresBehavior/UI/ui_handle.h"

namespace PlayerUi 
{
    enum class WidgetGroupID {
        None = 0,
        HealthBar,  // 体力バー
        AmmoCount,  // 弾薬数表示
        Crosshair,  // 銃の照準
        Max
    };
    /// @brief ウィジェットのグループを表す構造体
    struct WidgetGroup {
        std::vector<UiHandle> widgets;
        DirectX::XMFLOAT2 originalCenterPosition;
        DirectX::XMFLOAT2 currentCenterPosition;
        std::vector<DirectX::XMFLOAT2> offsetPositions;
    };
}

struct PlayerUiRuntimeState {
    
};

struct PlayerUiContext {
    class PlayerUiBehavior* owner = nullptr;
    class IScene* scene = nullptr;

    PlayerUiRuntimeState runtimeState;
    // settings

    class TransformComponent* cameraTransform = nullptr;
    class CameraComponent* cameraComponent = nullptr;
    class CameraControlBehavior* cameraControlBehavior = nullptr;

    class PlayerUiView* view = nullptr;
    class PlayerUiPresentation* presentation = nullptr;

    PlayerUi::WidgetGroup widgetGroups[static_cast<size_t>(PlayerUi::WidgetGroupID::Max)];
};
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
        DirectX::XMFLOAT2 originalCenterPosition = {}; // 演出前の基準位置
        DirectX::XMFLOAT2 currentCenterPosition = {};  // 演出反映後の位置
        DirectX::XMFLOAT2 shakeOffset = {};
        float currentAlpha = 1.0f;
        std::vector<DirectX::XMFLOAT2> offsetPositions;
    };
}

struct PlayerUiContext {
    class PlayerUiBehavior* owner = nullptr;
    class IScene* scene = nullptr;

    // settings

    class TransformComponent* cameraTransform = nullptr;
    class CameraComponent* cameraComponent = nullptr;
    class CameraControlBehavior* cameraControlBehavior = nullptr;

    class PlayerUiView* view = nullptr;
    class PlayerUiPresentation* presentation = nullptr;

    PlayerUi::WidgetGroup widgetGroups[static_cast<size_t>(PlayerUi::WidgetGroupID::Max)];
};

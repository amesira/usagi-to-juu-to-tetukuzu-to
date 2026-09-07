// player_ui_context.h
#pragma once
#include <vector>
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
        WidgetGroupID id = WidgetGroupID::None;
        std::vector<UiHandle> widgets;
    };

    struct PlayerUiContext {
        class PlayerUiBehavior* owner = nullptr;
        class IScene* scene = nullptr;

        class TransformComponent* cameraTransform = nullptr;
        class CameraComponent* cameraComponent = nullptr;
        class CameraControlBehavior* cameraControlBehavior = nullptr;
    };
}
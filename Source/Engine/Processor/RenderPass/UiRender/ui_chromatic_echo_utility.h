// ui_chromatic_echo_utility.h
// 2026/09/08
#pragma once
#include "ui_draw_command.h"
#include "Engine/Component/ui_chromatic_echo.h"
#include <algorithm>
#include <limits>
#include <utility>

namespace UiChromaticEchoUtility 
{
    using namespace DirectX;

    inline void Append(std::vector<UiDrawCommand::DrawBatch2D>& output, UiDrawCommand::DrawBatch2D batch, const UiChromaticEcho& echo) 
    {
        if (!echo.enabled || !echo.IsValid() || echo.opacity <= 0.0f || batch.instances.empty()) {
            output.push_back(std::move(batch));
            return;
        }

        UiDrawCommand::DrawBatch2D copy = batch;
        const int order = static_cast<int>(batch.orderInLayer) + echo.orderInLayerOffset;
        copy.orderInLayer = order;
        const XMMATRIX transform = XMMatrixTranslation(-echo.center.x, -echo.center.y, 0)
            * XMMatrixScaling(echo.scale, echo.scale, 1)
            * XMMatrixTranslation(echo.center.x + echo.offset.x, echo.center.y + echo.offset.y, 0);

        for (auto& instance : copy.instances) {
            XMStoreFloat4x4(&instance.presentationTransform,
                XMLoadFloat4x4(&instance.presentationTransform) * transform);
            instance.color = {echo.color.x, echo.color.y, echo.color.z, instance.color.w * echo.opacity};
        }

        // コピーと元のバッチを出力に追加
        output.push_back(std::move(copy));
        output.push_back(std::move(batch));
    }
}

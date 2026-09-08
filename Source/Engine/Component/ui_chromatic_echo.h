// ui_choromatic_echo.h
// 2026/09/08
// ・UIの描画において、クロマティックエコー（色収差）効果を適用するための構造体
#pragma once
#include <DirectXMath.h>
#include <cmath>

struct UiChromaticEcho {
    bool enabled = false;
    DirectX::XMFLOAT2 center = {};
    DirectX::XMFLOAT2 offset = {};
    float scale = 1.003f;
    DirectX::XMFLOAT3 color = {1, 0, 0};

    float opacity = 0.2f;
    int orderInLayerOffset = -10;

    bool IsValid() const {
        return std::isfinite(center.x) && std::isfinite(center.y)
            && std::isfinite(offset.x) && std::isfinite(offset.y)
            && std::isfinite(scale) && scale > 0
            && std::isfinite(opacity) && opacity >= 0 && opacity <= 1
            && std::isfinite(color.x) && color.x >= 0 && color.x <= 1
            && std::isfinite(color.y) && color.y >= 0 && color.y <= 1
            && std::isfinite(color.z) && color.z >= 0 && color.z <= 1;
    }
};
